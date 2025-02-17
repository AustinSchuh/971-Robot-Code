#ifndef AOS_NETWORK_SCTP_SERVER_H_
#define AOS_NETWORK_SCTP_SERVER_H_

#include <arpa/inet.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <sys/socket.h>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>

#include "aos/network/sctp_lib.h"
#include "aos/unique_malloc_ptr.h"
#include "glog/logging.h"

namespace aos {
namespace message_bridge {

class SctpServer {
 public:
  SctpServer(int streams, std::string_view local_host = "0.0.0.0",
             int local_port = 9971);

  ~SctpServer() {}

  // Receives the next packet from the remote.
  aos::unique_c_ptr<Message> Read() { return sctp_.ReadMessage(); }

  // Sends a block of data to a client on a stream with a TTL.  Returns true on
  // success.
  bool Send(std::string_view data, sctp_assoc_t snd_assoc_id, int stream,
            int time_to_live) {
    return sctp_.SendMessage(stream, data, time_to_live, std::nullopt,
                             snd_assoc_id);
  }

  // Aborts a connection.  Returns true on success.
  bool Abort(sctp_assoc_t snd_assoc_id);

  int fd() { return sctp_.fd(); }

  // Enables the priority scheduler.  This is a SCTP feature which lets us
  // configure the priority per stream so that higher priority packets don't get
  // backed up behind lower priority packets in the networking queues.
  void SetPriorityScheduler(sctp_assoc_t assoc_id);

  // Sets the priority of a specific stream.
  void SetStreamPriority(sctp_assoc_t assoc_id, int stream_id,
                         uint16_t priority);

  void SetMaxSize(size_t max_size) { sctp_.SetMaxSize(max_size); }

 private:
  struct sockaddr_storage sockaddr_local_;
  SctpReadWrite sctp_;
};

}  // namespace message_bridge
}  // namespace aos

#endif  // AOS_NETWORK_SCTP_SERVER_H_
