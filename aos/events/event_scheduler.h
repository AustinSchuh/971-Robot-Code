#ifndef AOS_EVENTS_EVENT_SCHEDULER_H_
#define AOS_EVENTS_EVENT_SCHEDULER_H_

#include <algorithm>
#include <map>
#include <memory>
#include <unordered_set>
#include <utility>
#include <vector>

#include "aos/events/event_loop.h"
#include "aos/events/logging/boot_timestamp.h"
#include "aos/logging/implementations.h"
#include "aos/time/time.h"
#include "glog/logging.h"

namespace aos {

// This clock is the basis for distributed time.  It is used to synchronize time
// between multiple nodes.  This is a new type so conversions to and from the
// monotonic and realtime clocks aren't implicit.
class distributed_clock {
 public:
  typedef ::std::chrono::nanoseconds::rep rep;
  typedef ::std::chrono::nanoseconds::period period;
  typedef ::std::chrono::nanoseconds duration;
  typedef ::std::chrono::time_point<distributed_clock> time_point;

  // This clock is the base clock for the simulation and everything is synced to
  // it.  It never jumps.
  static constexpr bool is_steady = true;

  // Returns the epoch (0).
  static constexpr time_point epoch() { return time_point(zero()); }

  static constexpr duration zero() { return duration(0); }

  static constexpr time_point min_time{
      time_point(duration(::std::numeric_limits<duration::rep>::min()))};
  static constexpr time_point max_time{
      time_point(duration(::std::numeric_limits<duration::rep>::max()))};
};

std::ostream &operator<<(std::ostream &stream,
                         const aos::distributed_clock::time_point &now);

// Interface to handle converting time on a node to and from the distributed
// clock accurately.
class TimeConverter {
 public:
  virtual ~TimeConverter() {}

  // Returns the boot UUID for a node and boot.  Note: the boot UUID for
  // subsequent calls needs to be the same each time.
  virtual UUID boot_uuid(size_t node_index, size_t boot_count) = 0;

  void set_reboot_found(
      std::function<void(distributed_clock::time_point,
                         const std::vector<logger::BootTimestamp> &)>
          fn) {
    reboot_found_ = fn;
  }

  // Converts a time to the distributed clock for scheduling and cross-node
  // time measurement.
  virtual distributed_clock::time_point ToDistributedClock(
      size_t node_index, logger::BootTimestamp time) = 0;

  // Takes the distributed time and converts it to the monotonic clock for this
  // node.
  virtual logger::BootTimestamp FromDistributedClock(
      size_t node_index, distributed_clock::time_point time,
      size_t boot_count) = 0;

  // Called whenever time passes this point and we can forget about it.
  virtual void ObserveTimePassed(distributed_clock::time_point time) = 0;

 protected:
  std::function<void(distributed_clock::time_point,
                     const std::vector<logger::BootTimestamp> &)>
      reboot_found_;
};

class EventSchedulerScheduler;

class EventScheduler {
 public:
  class Event {
   public:
    virtual void Handle() noexcept = 0;
    virtual ~Event() {}
  };

  using ChannelType = std::multimap<monotonic_clock::time_point, Event *>;
  using Token = ChannelType::iterator;
  EventScheduler(size_t node_index) : node_index_(node_index) {}

  // Sets the time converter in use for this scheduler (and the corresponding
  // node index)
  void SetTimeConverter(size_t node_index, TimeConverter *converter) {
    CHECK_EQ(node_index_, node_index);
    converter_ = converter;
  }

  UUID boot_uuid() { return converter_->boot_uuid(node_index_, boot_count_); }

  // Schedule an event with a callback function
  // Returns an iterator to the event
  Token Schedule(monotonic_clock::time_point time, Event *callback);

  // Schedules a callback when the event scheduler starts.
  void ScheduleOnRun(std::function<void()> callback) {
    on_run_.emplace_back(std::move(callback));
  }

  // Schedules a callback when the event scheduler starts.
  void ScheduleOnStartup(std::function<void()> callback) {
    on_startup_.emplace_back(std::move(callback));
  }

  void set_on_shutdown(std::function<void()> callback) {
    on_shutdown_ = std::move(callback);
  }

  void set_started(std::function<void()> callback) {
    started_ = std::move(callback);
  }

  std::function<void()> started_;
  std::function<void()> on_shutdown_;

  Token InvalidToken() { return events_list_.end(); }

  // Deschedule an event by its iterator
  void Deschedule(Token token);

  // Runs the OnRun callbacks.
  void RunOnRun();

  // Runs the OnStartup callbacks.
  void RunOnStartup();

  // Runs the Started callback.
  void RunStarted();

  // Returns true if events are being handled.
  inline bool is_running() const;

  // Returns the timestamp of the next event to trigger.
  monotonic_clock::time_point OldestEvent();
  // Handles the next event.
  void CallOldestEvent();

  // Converts a time to the distributed clock for scheduling and cross-node time
  // measurement.
  distributed_clock::time_point ToDistributedClock(
      monotonic_clock::time_point time) const {
    return converter_->ToDistributedClock(node_index_,
                                          {.boot = boot_count_, .time = time});
  }

  // Takes the distributed time and converts it to the monotonic clock for this
  // node.
  logger::BootTimestamp FromDistributedClock(
      distributed_clock::time_point time) const {
    return converter_->FromDistributedClock(node_index_, time, boot_count_);
  }

  // Returns the current monotonic time on this node calculated from the
  // distributed clock.
  inline monotonic_clock::time_point monotonic_now() const;

  // Returns the current monotonic time on this node calculated from the
  // distributed clock.
  inline distributed_clock::time_point distributed_now() const;

  size_t boot_count() const { return boot_count_; }

  size_t node_index() const { return node_index_; }

  // For implementing reboots.
  void Shutdown();
  void Startup();

 private:
  friend class EventSchedulerScheduler;

  // Current execution time.
  monotonic_clock::time_point monotonic_now_ = monotonic_clock::epoch();

  size_t boot_count_ = 0;

  // List of functions to run (once) when running.
  std::vector<std::function<void()>> on_run_;
  std::vector<std::function<void()>> on_startup_;

  // Multimap holding times to run functions.  These are stored in order, and
  // the order is the callback tree.
  ChannelType events_list_;

  // Pointer to the actual scheduler.
  EventSchedulerScheduler *scheduler_scheduler_ = nullptr;

  // Node index handle to be handed back to the TimeConverter.  This lets the
  // same time converter be used for all the nodes, and the node index
  // distinguish which one.
  size_t node_index_ = 0;

  // Converts time by doing nothing to it.
  class UnityConverter final : public TimeConverter {
   public:
    distributed_clock::time_point ToDistributedClock(
        size_t /*node_index*/, logger::BootTimestamp time) override {
      CHECK_EQ(time.boot, 0u) << ": Reboots unsupported by default.";
      return distributed_clock::epoch() + time.time.time_since_epoch();
    }

    logger::BootTimestamp FromDistributedClock(
        size_t /*node_index*/, distributed_clock::time_point time,
        size_t boot_count) override {
      CHECK_EQ(boot_count, 0u);
      return logger::BootTimestamp{
          .boot = boot_count,
          .time = monotonic_clock::epoch() + time.time_since_epoch()};
    }

    void ObserveTimePassed(distributed_clock::time_point /*time*/) override {}

    UUID boot_uuid(size_t /*node_index*/, size_t boot_count) override {
      CHECK_EQ(boot_count, 0u);
      return uuid_;
    }

   private:
    const UUID uuid_ = UUID::Random();
  };

  UnityConverter unity_converter_;

  TimeConverter *converter_ = &unity_converter_;
};

// We need a heap of heaps...
//
// Events in a node have a very well defined progression of time.  It is linear
// and well represented by the monotonic clock.
//
// Events across nodes don't follow this well.  Time skews between the two nodes
// all the time.  We also don't know the function ahead of time which converts
// from each node's monotonic clock to the distributed clock (our unified base
// time which is likely the average time between nodes).
//
// This pushes us towards merge sort.  Sorting each node's events with a heap
// like we used to be doing, and then sorting each of those nodes independently.
class EventSchedulerScheduler {
 public:
  // Adds an event scheduler to the list.
  void AddEventScheduler(EventScheduler *scheduler);

  // Runs until there are no more events or Exit is called.
  void Run();

  // Stops running.
  void Exit() { is_running_ = false; }

  bool is_running() const { return is_running_; }

  // Runs for a duration on the distributed clock.  Time on the distributed
  // clock should be very representative of time on each node, but won't be
  // exactly the same.
  void RunFor(distributed_clock::duration duration);

  // Returns the current distributed time.
  distributed_clock::time_point distributed_now() const { return now_; }

  void RunOnStartup() {
    CHECK(!is_running_);
    for (EventScheduler *scheduler : schedulers_) {
      scheduler->RunOnStartup();
    }
    for (EventScheduler *scheduler : schedulers_) {
      scheduler->RunStarted();
    }
  }

  void SetTimeConverter(TimeConverter *time_converter) {
    time_converter->set_reboot_found(
        [this](distributed_clock::time_point reboot_time,
               const std::vector<logger::BootTimestamp> &node_times) {
          if (!reboots_.empty()) {
            CHECK_GT(reboot_time, std::get<0>(reboots_.back()));
          }
          reboots_.emplace_back(reboot_time, node_times);
        });
  }

 private:
  // Handles running the OnRun functions.
  void RunOnRun() {
    CHECK(!is_running_);
    is_running_ = true;
    for (EventScheduler *scheduler : schedulers_) {
      scheduler->RunOnRun();
    }
  }

  void Reboot();

  // Returns the next event time and scheduler on which to run it.
  std::tuple<distributed_clock::time_point, EventScheduler *> OldestEvent();

  // True if we are running.
  bool is_running_ = false;
  // The current time.
  distributed_clock::time_point now_ = distributed_clock::epoch();
  // List of schedulers to run in sync.
  std::vector<EventScheduler *> schedulers_;

  // List of when to reboot each node.
  std::vector<std::tuple<distributed_clock::time_point,
                         std::vector<logger::BootTimestamp>>>
      reboots_;
};

inline distributed_clock::time_point EventScheduler::distributed_now() const {
  return scheduler_scheduler_->distributed_now();
}
inline monotonic_clock::time_point EventScheduler::monotonic_now() const {
  const logger::BootTimestamp t =
      FromDistributedClock(scheduler_scheduler_->distributed_now());
  CHECK_EQ(t.boot, boot_count_) << ": " << " " << t << " d "
                                << scheduler_scheduler_->distributed_now();
  return t.time;
}

inline bool EventScheduler::is_running() const {
  return scheduler_scheduler_->is_running();
}

}  // namespace aos

#endif  // AOS_EVENTS_EVENT_SCHEDULER_H_
