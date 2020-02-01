#include <iomanip>
#include <iostream>

#include "aos/events/logging/logger.h"
#include "aos/events/simulated_event_loop.h"
#include "aos/init.h"
#include "aos/json_to_flatbuffer.h"
#include "aos/time/time.h"
#include "gflags/gflags.h"

DEFINE_string(logfile, "/tmp/logfile.bfbs",
              "Name and path of the logfile to read from.");
DEFINE_string(
    name, "",
    "Name to match for printing out channels. Empty means no name filter.");

// define struct to hold all information
struct ChannelStats {
  // pointer to the channel for which stats are collected
  const aos::Channel *channel;
  aos::realtime_clock::time_point channel_end_time =
      aos::realtime_clock::min_time;
  aos::monotonic_clock::time_point first_message_time =
      // needs to be higher than time in the logfile!
      aos::monotonic_clock::max_time;
  aos::monotonic_clock::time_point current_message_time =
      aos::monotonic_clock::min_time;
  // channel stats to collect per channel
  int total_num_messages = 0;
  size_t max_message_size = 0;
  size_t total_message_size = 0;
  double avg_messages_sec = 0.0;  // TODO in Lambda, now in stats overview.
  double max_messages_sec = 0.0;  // TODO in Lambda
};

struct LogfileStats {
  // All relevant stats on to logfile level
  size_t logfile_length = 0;
  int total_log_messages = 0;
  aos::realtime_clock::time_point logfile_end_time =
      aos::realtime_clock::min_time;
};

int main(int argc, char **argv) {
  gflags::SetUsageMessage(
      "This program provides statistics on a given log file. Supported "
      "statistics are:\n"
      " - Logfile start time;\n"
      " - Total messages per channel/type;\n"
      " - Max message size per channel/type;\n"
      " - Frequency of messages per second;\n"
      " - Total logfile size and number of messages.\n"
      "Use --logfile flag to select a logfile (path/filename) and use --name "
      "flag to specify a channel to listen on.");

  aos::InitGoogle(&argc, &argv);

  LogfileStats logfile_stats;
  std::vector<ChannelStats> channel_stats;

  // Open LogFile
  aos::logger::LogReader reader(FLAGS_logfile);
  aos::SimulatedEventLoopFactory log_reader_factory(reader.configuration());
  reader.Register(&log_reader_factory);

  // Make an eventloop for retrieving stats
  std::unique_ptr<aos::EventLoop> stats_event_loop =
      log_reader_factory.MakeEventLoop("logstats", reader.node());
  stats_event_loop->SkipTimingReport();

  // Read channel info and store in vector
  bool found_channel = false;
  const flatbuffers::Vector<flatbuffers::Offset<aos::Channel>> *channels =
      reader.configuration()->channels();

  int it = 0;  // iterate through the channel_stats
  for (flatbuffers::uoffset_t i = 0; i < channels->size(); i++) {
    const aos::Channel *channel = channels->Get(i);
    if (channel->name()->string_view().find(FLAGS_name) != std::string::npos) {
      // Add a record to the stats vector.
      channel_stats.push_back({channel});
      // Lambda to read messages and parse for information
      stats_event_loop->MakeRawWatcher(
          channel,
          [&logfile_stats, &channel_stats, it](const aos::Context &context,
                                               const void * /* message */) {
            channel_stats[it].max_message_size =
                std::max(channel_stats[it].max_message_size, context.size);
            channel_stats[it].total_message_size += context.size;
            channel_stats[it].total_num_messages++;
            // asume messages are send in sequence per channel
            channel_stats[it].channel_end_time = context.realtime_event_time;
            channel_stats[it].first_message_time =
                std::min(channel_stats[it].first_message_time,
                         context.monotonic_event_time);
            channel_stats[it].current_message_time =
                context.monotonic_event_time;
            // update the overall logfile statistics
            logfile_stats.logfile_length += context.size;
          });
      it++;
      // TODO (Stephan): Frequency of messages per second
      // - Sliding window
      // - Max / Deviation
      found_channel = true;
    }
  }
  if (!found_channel) {
    LOG(FATAL) << "Could not find any channels";
  }

  log_reader_factory.Run();

  // Print out the stats per channel and for the logfile
  for (size_t i = 0; i != channel_stats.size(); i++) {
    if (channel_stats[i].total_num_messages > 0) {
      double sec_active =
          aos::time::DurationInSeconds(channel_stats[i].current_message_time -
                                       channel_stats[i].first_message_time);
      channel_stats[i].avg_messages_sec =
          (channel_stats[i].total_num_messages / sec_active);
      logfile_stats.total_log_messages += channel_stats[i].total_num_messages;
      logfile_stats.logfile_end_time = std::max(
          logfile_stats.logfile_end_time, channel_stats[i].channel_end_time);
      std::cout << "Channel name: "
                << channel_stats[i].channel->name()->string_view()
                << "\tMsg type: "
                << channel_stats[i].channel->type()->string_view() << "\n"
                << "Number of msg: " << channel_stats[i].total_num_messages
                << std::setprecision(3) << std::fixed
                << "\tAvg msg per sec: " << channel_stats[i].avg_messages_sec
                << "\tSet max msg frequency: "
                << channel_stats[i].channel->frequency() << "\n"
                << "Avg msg size: "
                << (channel_stats[i].total_message_size /
                    channel_stats[i].total_num_messages)
                << "\tMax msg size: " << channel_stats[i].max_message_size
                << "\tSet max msg size: "
                << channel_stats[i].channel->max_size() << "\n"
                << "First msg time: " << channel_stats[i].first_message_time
                << "\tLast msg time: " << channel_stats[i].current_message_time
                << "\tSeconds active: " << sec_active << "sec\n";
    } else {
      std::cout << "Channel name: "
                << channel_stats[i].channel->name()->string_view() << "\t"
                << "Msg type: "
                << channel_stats[i].channel->type()->string_view() << "\n"
                << "Set max msg frequency: "
                << channel_stats[i].channel->frequency() << "\t"
                << "Set max msg size: " << channel_stats[i].channel->max_size()
                << "\n--- No messages in channel ---"
                << "\n";
    }
  }
  std::cout << std::setfill('-') << std::setw(80) << "-"
            << "\nLogfile statistics for: " << FLAGS_logfile << "\n"
            << "Log starts at:\t" << reader.realtime_start_time() << "\n"
            << "Log ends at:\t" << logfile_stats.logfile_end_time << "\n"
            << "Log file size:\t" << logfile_stats.logfile_length << "\n"
            << "Total messages:\t" << logfile_stats.total_log_messages << "\n";

  // Cleanup the created processes
  reader.Deregister();
  aos::Cleanup();
  return 0;
}
