#include <iostream>

#include "aos/configuration.h"
#include "aos/events/logging/log_reader.h"
#include "aos/events/simulated_event_loop.h"
#include "aos/init.h"
#include "aos/json_to_flatbuffer.h"
#include "aos/util/file.h"
#include "gflags/gflags.h"

DEFINE_string(logfile, "/tmp/logfile.bfbs",
              "Name of the logfile to read from.");
DEFINE_bool(
    replace, false,
    "If true, replace the header on the log file with the JSON header.");
DEFINE_string(
    header, "",
    "If provided, this is the path to the JSON with the log file header.  If "
    "not provided, _header.json will be appended to --logfile.");

int main(int argc, char **argv) {
  gflags::SetUsageMessage(R"(This tool lets us manipulate log files.)");
  aos::InitGoogle(&argc, &argv);

  std::string header =
      FLAGS_header.empty() ? (FLAGS_logfile + "_header.json") : FLAGS_header;

  if (FLAGS_replace) {
    const ::std::string header_json = aos::util::ReadFileToStringOrDie(header);
    flatbuffers::FlatBufferBuilder fbb;
    fbb.ForceDefaults(true);
    flatbuffers::Offset<aos::logger::LogFileHeader> header =
        aos::JsonToFlatbuffer<aos::logger::LogFileHeader>(header_json, &fbb);

    fbb.FinishSizePrefixed(header);

    const std::string orig_path = FLAGS_logfile + ".orig";
    PCHECK(rename(FLAGS_logfile.c_str(), orig_path.c_str()) == 0);

    aos::logger::SpanReader span_reader(orig_path);
    CHECK(!span_reader.ReadMessage().empty()) << ": Empty header, aborting";

    aos::logger::DetachedBufferWriter buffer_writer(
        FLAGS_logfile, std::make_unique<aos::logger::DummyEncoder>());
    buffer_writer.QueueSizedFlatbuffer(&fbb, aos::monotonic_clock::min_time);

    while (true) {
      absl::Span<const uint8_t> msg_data = span_reader.ReadMessage();
      if (msg_data == absl::Span<const uint8_t>()) {
        break;
      }

      buffer_writer.QueueSpan(msg_data);
    }
  } else {
    aos::logger::MessageReader reader(FLAGS_logfile);
    aos::util::WriteStringToFileOrDie(
        header,
        aos::FlatbufferToJson(reader.log_file_header(), {.multi_line = true}));
  }

  return 0;
}
