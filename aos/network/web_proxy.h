#ifndef AOS_NETWORK_WEB_PROXY_H_
#define AOS_NETWORK_WEB_PROXY_H_
#include <map>
#include <set>
#include "aos/seasocks/seasocks_logger.h"
#include "flatbuffers/flatbuffers.h"
#include "seasocks/Server.h"
#include "seasocks/StringUtil.h"
#include "seasocks/WebSocket.h"

#include "api/peer_connection_interface.h"

namespace aos {
namespace web_proxy {

class Connection;

// Basic class that handles receiving new websocket connections. Creates a new
// Connection to manage the rest of the negotiation and data passing. When the
// websocket closes, it deletes the Connection.
class WebsocketHandler : public ::seasocks::WebSocket::Handler {
 public:
  WebsocketHandler(::seasocks::Server *server);
  void onConnect(::seasocks::WebSocket *sock) override;
  void onData(::seasocks::WebSocket *sock, const uint8_t *data,
              size_t size) override;
  void onDisconnect(::seasocks::WebSocket *sock) override;

 private:
  std::map<::seasocks::WebSocket *, std::unique_ptr<Connection>> connections_;
  ::seasocks::Server *server_;
};

// Seasocks requires that sends happen on the correct thread. This class takes a
// detached buffer to send on a specific websocket connection and sends it when
// seasocks is ready.
class UpdateData : public ::seasocks::Server::Runnable {
 public:
  UpdateData(::seasocks::WebSocket *websocket,
             ::flatbuffers::DetachedBuffer &&buffer)
      : sock_(websocket), buffer_(std::move(buffer)) {}
  ~UpdateData() override = default;
  UpdateData(const UpdateData &) = delete;
  UpdateData &operator=(const UpdateData &) = delete;

  void run() override { sock_->send(buffer_.data(), buffer_.size()); }

 private:
  ::seasocks::WebSocket *sock_;
  const ::flatbuffers::DetachedBuffer buffer_;
};

// Represents a single connection to a browser for the entire lifetime of the
// connection.
class Connection : public webrtc::PeerConnectionObserver,
                   public webrtc::CreateSessionDescriptionObserver,
                   public webrtc::DataChannelObserver {
 public:
  Connection(::seasocks::WebSocket *sock, ::seasocks::Server *server);

  void HandleWebSocketData(const uint8_t *data, size_t size);

  // PeerConnectionObserver implementation
  void OnSignalingChange(
      webrtc::PeerConnectionInterface::SignalingState) override {}
  void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface>) override {}
  void OnRemoveStream(
      rtc::scoped_refptr<webrtc::MediaStreamInterface>) override {}
  void OnDataChannel(
      rtc::scoped_refptr<webrtc::DataChannelInterface> channel) override;
  void OnRenegotiationNeeded() override {}
  void OnIceConnectionChange(
      webrtc::PeerConnectionInterface::IceConnectionState state) override {}
  void OnIceGatheringChange(
      webrtc::PeerConnectionInterface::IceGatheringState) override {}
  void OnIceCandidate(const webrtc::IceCandidateInterface *candidate) override;
  void OnIceConnectionReceivingChange(bool) override {}

  // CreateSessionDescriptionObserver implementation
  void OnSuccess(webrtc::SessionDescriptionInterface *desc) override;
  void OnFailure(webrtc::RTCError error) override {}
  // CreateSessionDescriptionObserver is a refcounted object
  void AddRef() const override {}
  // We handle ownership with a unique_ptr so don't worry about actually
  // refcounting. We will delete when we are done.
  rtc::RefCountReleaseStatus Release() const override {
    return rtc::RefCountReleaseStatus::kOtherRefsRemained;
  }

  // DataChannelObserver implementation
  void OnStateChange() override {}
  void OnMessage(const webrtc::DataBuffer &buffer) override;
  void OnBufferedAmountChange(uint64_t sent_data_size) override {}

 private:
  ::seasocks::WebSocket *sock_;
  ::seasocks::Server *server_;
  rtc::scoped_refptr<webrtc::PeerConnectionInterface> peer_connection_;
  rtc::scoped_refptr<webrtc::DataChannelInterface> data_channel_;
};

}  // namespace web_proxy
}  // namespace aos

#endif  // AOS_NETWORK_WEB_PROXY_H_
