#include "aos/ipc_lib/queue_racer.h"

#include <cinttypes>
#include <cstring>
#include <limits>

#include "aos/ipc_lib/event.h"
#include "gtest/gtest.h"

namespace aos {
namespace ipc_lib {
namespace {

struct ThreadPlusCount {
  int thread;
  uint64_t count;
};

}  // namespace

struct ThreadState {
  ::std::thread thread;
  Event ready;
  uint64_t event_count = ::std::numeric_limits<uint64_t>::max();
};

QueueRacer::QueueRacer(LocklessQueue queue, int num_threads,
                       uint64_t num_messages)
    : queue_(queue), num_threads_(num_threads), num_messages_(num_messages) {
  Reset();
}

void QueueRacer::RunIteration(bool race_reads, int write_wrap_count) {
  const bool will_wrap = num_messages_ * num_threads_ *
                             static_cast<uint64_t>(1 + write_wrap_count) >
                         queue_.config().queue_size;

  // Clear out shmem.
  Reset();
  started_writes_ = 0;
  finished_writes_ = 0;

  // Event used to start all the threads processing at once.
  Event run;

  ::std::atomic<bool> poll_index{true};

  // List of threads.
  ::std::vector<ThreadState> threads(num_threads_);

  ::std::thread queue_index_racer([this, &poll_index]() {
    LocklessQueueReader reader(queue_);

    // Track the number of times we wrap, and cache the modulo.
    uint64_t wrap_count = 0;
    uint32_t last_queue_index = 0;
    const uint32_t max_queue_index =
        QueueIndex::MaxIndex(0xffffffffu, queue_.config().queue_size);
    while (poll_index) {
      // We want to read everything backwards.  This will give us conservative
      // bounds.  And with enough time and randomness, we will see all the cases
      // we care to see.

      // These 3 numbers look at the same thing, but at different points of time
      // in the process.  The process (essentially) looks like:
      //
      // ++started_writes;
      // ++latest_queue_index;
      // ++finished_writes;
      //
      // We want to check that latest_queue_index is bounded by the number of
      // writes started and finished.  Basically, we can say that
      // finished_writes < latest_queue_index always.  And
      // latest_queue_index < started_writes.  And everything always increases.
      // So, if we let more time elapse between sampling finished_writes and
      // latest_queue_index, we will only be relaxing our bounds, not
      // invalidating the check.  The same goes for started_writes.
      //
      // So, grab them in order.
      const uint64_t finished_writes = finished_writes_.load();
      const QueueIndex latest_queue_index_queue_index = reader.LatestIndex();
      const uint64_t started_writes = started_writes_.load();

      const uint32_t latest_queue_index_uint32_t =
          latest_queue_index_queue_index.index();
      uint64_t latest_queue_index = latest_queue_index_uint32_t;

      if (latest_queue_index_queue_index != QueueIndex::Invalid()) {
        // If we got smaller, we wrapped.
        if (latest_queue_index_uint32_t < last_queue_index) {
          ++wrap_count;
        }
        // And apply it.
        latest_queue_index +=
            static_cast<uint64_t>(max_queue_index) * wrap_count;
        last_queue_index = latest_queue_index_uint32_t;
      }

      // For grins, check that we have always started more than we finished.
      // Should never fail.
      EXPECT_GE(started_writes, finished_writes);

      // If we are at the beginning, the queue needs to always return empty.
      if (started_writes == 0) {
        EXPECT_EQ(latest_queue_index_queue_index, QueueIndex::Invalid());
        EXPECT_EQ(finished_writes, 0);
      } else {
        if (finished_writes == 0) {
          // Plausible to be at the beginning, in which case we don't have
          // anything to check.
          if (latest_queue_index_queue_index != QueueIndex::Invalid()) {
            // Otherwise, we have started.  The queue can't have any more
            // entries than this.
            EXPECT_GE(started_writes, latest_queue_index + 1);
          }
        } else {
          EXPECT_NE(latest_queue_index_queue_index, QueueIndex::Invalid());
          // latest_queue_index is an index, not a count.  So it always reads 1
          // low.
          EXPECT_GE(latest_queue_index + 1, finished_writes);
        }
      }
    }
  });

  // Build up each thread and kick it off.
  int thread_index = 0;
  for (ThreadState &t : threads) {
    if (will_wrap) {
      t.event_count = ::std::numeric_limits<uint64_t>::max();
    } else {
      t.event_count = 0;
    }
    t.thread = ::std::thread([this, &t, thread_index, &run,
                              write_wrap_count]() {
      // Build up a sender.
      LocklessQueueSender sender = LocklessQueueSender::Make(queue_).value();
      CHECK_GE(sender.size(), sizeof(ThreadPlusCount));

      // Signal that we are ready to start sending.
      t.ready.Set();

      // Wait until signaled to start running.
      run.Wait();

      // Gogogo!
      for (uint64_t i = 0;
           i < num_messages_ * static_cast<uint64_t>(1 + write_wrap_count);
           ++i) {
        char *const data = static_cast<char *>(sender.Data()) + sender.size() -
                           sizeof(ThreadPlusCount);
        const char fill = (i + 55) & 0xFF;
        memset(data, fill, sizeof(ThreadPlusCount));
        {
          bool found_nonzero = false;
          for (size_t i = 0; i < sizeof(ThreadPlusCount); ++i) {
            if (data[i] != fill) {
              found_nonzero = true;
            }
          }
          CHECK(!found_nonzero) << ": Somebody else is writing to our buffer";
        }

        ThreadPlusCount tpc;
        tpc.thread = thread_index;
        tpc.count = i;

        memcpy(data, &tpc, sizeof(ThreadPlusCount));

        if (i % 0x800000 == 0x100000) {
          fprintf(
              stderr, "Sent %" PRIu64 ", %f %%\n", i,
              static_cast<double>(i) /
                  static_cast<double>(num_messages_ * (1 + write_wrap_count)) *
                  100.0);
        }

        ++started_writes_;
        sender.Send(sizeof(ThreadPlusCount), aos::monotonic_clock::min_time,
                    aos::realtime_clock::min_time, 0xffffffff, UUID::Zero(),
                    nullptr, nullptr, nullptr);
        // Blank out the new scratch buffer, to catch other people using it.
        {
          char *const new_data = static_cast<char *>(sender.Data()) +
                                 sender.size() - sizeof(ThreadPlusCount);
          const char new_fill = ~fill;
          memset(new_data, new_fill, sizeof(ThreadPlusCount));
        }
        ++finished_writes_;
      }
    });
    ++thread_index;
  }

  // Wait until all the threads are ready.
  for (ThreadState &t : threads) {
    t.ready.Wait();
  }

  // And start them racing.
  run.Set();

  // Let all the threads finish before reading if we are supposed to not be
  // racing reads.
  if (!race_reads) {
    for (ThreadState &t : threads) {
      t.thread.join();
    }
    poll_index = false;
    queue_index_racer.join();
  }

  CheckReads(race_reads, write_wrap_count, &threads);

  // Reap all the threads.
  if (race_reads) {
    for (ThreadState &t : threads) {
      t.thread.join();
    }
    poll_index = false;
    queue_index_racer.join();
  }

  // Confirm that the number of writes matches the expected number of writes.
  ASSERT_EQ(num_threads_ * num_messages_ * (1 + write_wrap_count),
            started_writes_);
  ASSERT_EQ(num_threads_ * num_messages_ * (1 + write_wrap_count),
            finished_writes_);

  // And that every thread sent the right number of messages.
  for (ThreadState &t : threads) {
    if (will_wrap) {
      if (!race_reads) {
        // If we are wrapping, there is a possibility that a thread writes
        // everything *before* we can read any of it, and it all gets
        // overwritten.
        ASSERT_TRUE(t.event_count == ::std::numeric_limits<uint64_t>::max() ||
                    t.event_count == (1 + write_wrap_count) * num_messages_)
            << ": Got " << t.event_count << " events, expected "
            << (1 + write_wrap_count) * num_messages_;
      }
    } else {
      ASSERT_EQ(t.event_count, num_messages_);
    }
  }
}

void QueueRacer::CheckReads(bool race_reads, int write_wrap_count,
                            ::std::vector<ThreadState> *threads) {
  // Now read back the results to double check.
  LocklessQueueReader reader(queue_);
  const bool will_wrap = num_messages_ * num_threads_ * (1 + write_wrap_count) >
                         LocklessQueueSize(queue_.memory());

  monotonic_clock::time_point last_monotonic_sent_time =
      monotonic_clock::epoch();
  uint64_t initial_i = 0;
  if (will_wrap) {
    initial_i = (1 + write_wrap_count) * num_messages_ * num_threads_ -
                LocklessQueueSize(queue_.memory());
  }

  for (uint64_t i = initial_i;
       i < (1 + write_wrap_count) * num_messages_ * num_threads_; ++i) {
    monotonic_clock::time_point monotonic_sent_time;
    realtime_clock::time_point realtime_sent_time;
    monotonic_clock::time_point monotonic_remote_time;
    realtime_clock::time_point realtime_remote_time;
    UUID source_boot_uuid;
    uint32_t remote_queue_index;
    size_t length;
    char read_data[1024];

    // Handle overflowing the message count for the wrap test.
    const uint32_t wrapped_i =
        i % static_cast<size_t>(QueueIndex::MaxIndex(
                0xffffffffu, LocklessQueueSize(queue_.memory())));
    LocklessQueueReader::Result read_result = reader.Read(
        wrapped_i, &monotonic_sent_time, &realtime_sent_time,
        &monotonic_remote_time, &realtime_remote_time, &remote_queue_index,
        &source_boot_uuid, &length, &(read_data[0]));

    if (race_reads) {
      if (read_result == LocklessQueueReader::Result::NOTHING_NEW) {
        --i;
        continue;
      }
    }

    if (race_reads && will_wrap) {
      if (read_result == LocklessQueueReader::Result::TOO_OLD) {
        continue;
      }
    }
    // Every message should be good.
    ASSERT_EQ(read_result, LocklessQueueReader::Result::GOOD) << ": i is " << i;

    // And, confirm that time never went backwards.
    ASSERT_GT(monotonic_sent_time, last_monotonic_sent_time);
    last_monotonic_sent_time = monotonic_sent_time;

    EXPECT_EQ(monotonic_remote_time, aos::monotonic_clock::min_time);
    EXPECT_EQ(realtime_remote_time, aos::realtime_clock::min_time);
    EXPECT_EQ(source_boot_uuid, UUID::Zero());

    ThreadPlusCount tpc;
    ASSERT_EQ(length, sizeof(ThreadPlusCount));
    memcpy(&tpc,
           read_data + LocklessQueueMessageDataSize(queue_.memory()) - length,
           sizeof(ThreadPlusCount));

    if (will_wrap) {
      // The queue won't chang out from under us, so we should get some amount
      // of the tail end of the messages from a a thread.
      // Confirm that once we get our first message, they all show up.
      if ((*threads)[tpc.thread].event_count ==
          ::std::numeric_limits<uint64_t>::max()) {
        (*threads)[tpc.thread].event_count = tpc.count;
      }

      if (race_reads) {
        // Make sure nothing goes backwards.  Really not much we can do here.
        ASSERT_LE((*threads)[tpc.thread].event_count, tpc.count)
            << ": Thread " << tpc.thread;
        (*threads)[tpc.thread].event_count = tpc.count;
      } else {
        // Make sure nothing goes backwards.  Really not much we can do here.
        ASSERT_EQ((*threads)[tpc.thread].event_count, tpc.count)
            << ": Thread " << tpc.thread;
      }
    } else {
      // Confirm that we see every message counter from every thread.
      ASSERT_EQ((*threads)[tpc.thread].event_count, tpc.count)
          << ": Thread " << tpc.thread;
    }
    ++(*threads)[tpc.thread].event_count;
  }
}

}  // namespace ipc_lib
}  // namespace aos
