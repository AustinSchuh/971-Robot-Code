#include <sys/resource.h>
#include <sys/time.h>

#include "aos/configuration.h"
#include "aos/events/logging/log_writer.h"
#include "aos/events/logging/snappy_encoder.h"
#include "aos/events/shm_event_loop.h"
#include "aos/init.h"
#include "aos/logging/log_namer.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

DEFINE_string(config, "config.json", "Config file to use.");

DEFINE_bool(skip_renicing, false,
            "If true, skip renicing the logger.  This leaves it lower priority "
            "and increases the likelihood of dropping messages and crashing.");

DEFINE_bool(snappy_compress, false, "If true, compress log data using snappy.");

int main(int argc, char *argv[]) {
  gflags::SetUsageMessage(
      "This program provides a simple logger binary that logs all SHMEM data "
      "directly to a file specified at the command line. It does not manage "
      "filenames, so it will just crash if you attempt to overwrite an "
      "existing file, and the user must specify the logfile manually at the "
      "command line.");
  aos::InitGoogle(&argc, &argv);

  aos::FlatbufferDetachedBuffer<aos::Configuration> config =
      aos::configuration::ReadConfig(FLAGS_config);

  aos::ShmEventLoop event_loop(&config.message());

  std::unique_ptr<aos::logger::MultiNodeLogNamer> log_namer;
  log_namer = std::make_unique<aos::logger::MultiNodeLogNamer>(
      absl::StrCat(aos::logging::GetLogName("fbs_log"), "/"), &event_loop);

  if (FLAGS_snappy_compress) {
    log_namer->set_extension(aos::logger::SnappyDecoder::kExtension);
    log_namer->set_encoder_factory(
        []() { return std::make_unique<aos::logger::SnappyEncoder>(); });
  }

  aos::logger::Logger logger(&event_loop);
  event_loop.OnRun([&log_namer, &logger]() {
    if (FLAGS_skip_renicing) {
      LOG(WARNING) << "Ignoring request to renice to -20 due to "
                      "--skip_renicing.";
    } else {
      errno = 0;
      setpriority(PRIO_PROCESS, 0, -20);
      PCHECK(errno == 0)
          << ": Renicing to -20 failed, use --skip_renicing to skip renicing.";
    }
    logger.StartLogging(std::move(log_namer));
  });

  event_loop.Run();

  LOG(INFO) << "Shutting down";

  return 0;
}
