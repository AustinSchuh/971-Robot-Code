#include <unistd.h>

#include <iostream>

#include "aos/aos_cli_utils.h"
#include "aos/configuration.h"
#include "aos/init.h"
#include "aos/json_to_flatbuffer.h"
#include "gflags/gflags.h"
#include "glog/logging.h"

int main(int argc, char **argv) {
  gflags::SetUsageMessage(
      "Sends messages on arbitrary channels.\n"
      "Typical Usage: aos_send [--config path_to_config.json]"
      " channel_name message_type '{\"foo\": \"bar\"}'\n"
      "Example usage: aos_send /test aos.examples.Ping "
      "'{\"value\": 1}'");
  aos::InitGoogle(&argc, &argv);

  aos::CliUtilInfo cli_info;
  if (cli_info.Initialize(
          &argc, &argv,
          [&cli_info](const aos::Channel *channel) {
            return aos::configuration::ChannelIsSendableOnNode(
                channel, cli_info.event_loop->node());
          },
          false)) {
    return 0;
  }
  if (cli_info.found_channels.size() > 1) {
    LOG(FATAL) << "Matched multiple channels, but may only send on 1";
  }

  if (argc == 1) {
    LOG(FATAL) << "Must specify a message to send";
  }

  const aos::Channel *const channel = cli_info.found_channels[0];
  const std::unique_ptr<aos::RawSender> sender =
      cli_info.event_loop->MakeRawSender(channel);
  flatbuffers::FlatBufferBuilder fbb(sender->fbb_allocator()->size(),
                                     sender->fbb_allocator());
  fbb.ForceDefaults(true);
  fbb.Finish(aos::JsonToFlatbuffer(std::string_view(argv[1]), channel->schema(),
                                   &fbb));
  sender->CheckOk(sender->Send(fbb.GetSize()));

  return 0;
}
