#include "aos/network/message_bridge_client_lib.h"

#include "aos/events/shm_event_loop.h"
#include "aos/init.h"

DEFINE_string(config, "config.json", "Path to the config.");
DEFINE_int32(rt_priority, -1, "If > 0, run as this RT priority");

namespace aos {
namespace message_bridge {

int Main() {
  aos::FlatbufferDetachedBuffer<aos::Configuration> config =
      aos::configuration::ReadConfig(FLAGS_config);

  aos::ShmEventLoop event_loop(&config.message());
  if (FLAGS_rt_priority > 0) {
    event_loop.SetRuntimeRealtimePriority(FLAGS_rt_priority);
  }

  MessageBridgeClient app(&event_loop);

  // TODO(austin): Save messages into a vector to be logged.  One file per
  // channel?  Need to sort out ordering.
  //
  // TODO(austin): Low priority, "reliable" logging channel.

  event_loop.Run();

  return EXIT_SUCCESS;
}

}  // namespace message_bridge
}  // namespace aos

int main(int argc, char **argv) {
  aos::InitGoogle(&argc, &argv);

  return aos::message_bridge::Main();
}
