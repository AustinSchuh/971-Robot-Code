#include "aos/events/simulated_event_loop.h"

#include <algorithm>
#include <deque>
#include <string_view>

#include "absl/container/btree_map.h"
#include "aos/events/simulated_network_bridge.h"
#include "aos/json_to_flatbuffer.h"
#include "aos/util/phased_loop.h"

namespace aos {

// Container for both a message, and the context for it for simulation.  This
// makes tracking the timestamps associated with the data easy.
struct SimulatedMessage {
  // Context for the data.
  Context context;

  // The data.
  char *data(size_t buffer_size) {
    return RoundChannelData(&actual_data[0], buffer_size);
  }

  // Then the data, including padding on the end so we can align the buffer we
  // actually return from data().
  char actual_data[];
};

class SimulatedEventLoop;
class SimulatedFetcher;
class SimulatedChannel;

class SimulatedWatcher : public WatcherState {
 public:
  SimulatedWatcher(
      SimulatedEventLoop *simulated_event_loop, EventScheduler *scheduler,
      NodeEventLoopFactory *node_event_loop_factory,
      const Channel *channel,
      std::function<void(const Context &context, const void *message)> fn);

  ~SimulatedWatcher() override;

  void Startup(EventLoop * /*event_loop*/) override {}

  void Schedule(std::shared_ptr<SimulatedMessage> message);

  void HandleEvent();

  void SetSimulatedChannel(SimulatedChannel *channel) {
    simulated_channel_ = channel;
  }

 private:
  void DoSchedule(monotonic_clock::time_point event_time);

  ::std::deque<std::shared_ptr<SimulatedMessage>> msgs_;

  SimulatedEventLoop *simulated_event_loop_;
  EventHandler<SimulatedWatcher> event_;
  EventScheduler *scheduler_;
  NodeEventLoopFactory *node_event_loop_factory_;
  EventScheduler::Token token_;
  SimulatedChannel *simulated_channel_ = nullptr;
};

class SimulatedChannel {
 public:
  explicit SimulatedChannel(const Channel *channel, EventScheduler *scheduler)
      : channel_(channel),
        scheduler_(scheduler),
        next_queue_index_(ipc_lib::QueueIndex::Zero(channel->max_size())) {}

  ~SimulatedChannel() { CHECK_EQ(0u, fetchers_.size()); }

  // Makes a connected raw sender which calls Send below.
  ::std::unique_ptr<RawSender> MakeRawSender(EventLoop *event_loop);

  // Makes a connected raw fetcher.
  ::std::unique_ptr<RawFetcher> MakeRawFetcher(EventLoop *event_loop);

  // Registers a watcher for the queue.
  void MakeRawWatcher(SimulatedWatcher *watcher);

  void RemoveWatcher(SimulatedWatcher *watcher) {
    watchers_.erase(std::find(watchers_.begin(), watchers_.end(), watcher));
  }

  // Sends the message to all the connected receivers and fetchers.  Returns the
  // sent queue index.
  uint32_t Send(std::shared_ptr<SimulatedMessage> message);

  // Unregisters a fetcher.
  void UnregisterFetcher(SimulatedFetcher *fetcher);

  std::shared_ptr<SimulatedMessage> latest_message() { return latest_message_; }

  size_t max_size() const { return channel()->max_size(); }

  const std::string_view name() const {
    return channel()->name()->string_view();
  }

  const Channel *channel() const { return channel_; }

 private:
  const Channel *channel_;

  // List of all watchers.
  ::std::vector<SimulatedWatcher *> watchers_;

  // List of all fetchers.
  ::std::vector<SimulatedFetcher *> fetchers_;
  std::shared_ptr<SimulatedMessage> latest_message_;
  EventScheduler *scheduler_;

  ipc_lib::QueueIndex next_queue_index_;
};

namespace {

// Creates a SimulatedMessage with size bytes of storage.
// This is a shared_ptr so we don't have to implement refcounting or copying.
std::shared_ptr<SimulatedMessage> MakeSimulatedMessage(size_t size) {
  SimulatedMessage *message = reinterpret_cast<SimulatedMessage *>(
      malloc(sizeof(SimulatedMessage) + size + kChannelDataAlignment - 1));
  message->context.size = size;
  message->context.data = message->data(size);

  return std::shared_ptr<SimulatedMessage>(message, free);
}

class SimulatedSender : public RawSender {
 public:
  SimulatedSender(SimulatedChannel *simulated_channel, EventLoop *event_loop)
      : RawSender(event_loop, simulated_channel->channel()),
        simulated_channel_(simulated_channel),
        event_loop_(event_loop) {}
  ~SimulatedSender() {}

  void *data() override {
    if (!message_) {
      message_ = MakeSimulatedMessage(simulated_channel_->max_size());
    }
    return message_->data(simulated_channel_->max_size());
  }

  size_t size() override { return simulated_channel_->max_size(); }

  bool DoSend(size_t length,
              aos::monotonic_clock::time_point monotonic_remote_time,
              aos::realtime_clock::time_point realtime_remote_time,
              uint32_t remote_queue_index) override {
    CHECK_LE(length, size()) << ": Attempting to send too big a message.";
    message_->context.monotonic_event_time = event_loop_->monotonic_now();
    message_->context.monotonic_remote_time = monotonic_remote_time;
    message_->context.remote_queue_index = remote_queue_index;
    message_->context.realtime_event_time = event_loop_->realtime_now();
    message_->context.realtime_remote_time = realtime_remote_time;
    CHECK_LE(length, message_->context.size);
    message_->context.size = length;

    // TODO(austin): Track sending too fast.
    sent_queue_index_ = simulated_channel_->Send(message_);
    monotonic_sent_time_ = event_loop_->monotonic_now();
    realtime_sent_time_ = event_loop_->realtime_now();

    // Drop the reference to the message so that we allocate a new message for
    // next time.  Otherwise we will continue to reuse the same memory for all
    // messages and corrupt it.
    message_.reset();
    return true;
  }

  bool DoSend(const void *msg, size_t size,
              aos::monotonic_clock::time_point monotonic_remote_time,
              aos::realtime_clock::time_point realtime_remote_time,
              uint32_t remote_queue_index) override {
    CHECK_LE(size, this->size()) << ": Attempting to send too big a message.";

    // This is wasteful, but since flatbuffers fill from the back end of the
    // queue, we need it to be full sized.
    message_ = MakeSimulatedMessage(simulated_channel_->max_size());

    // Now fill in the message.  size is already populated above, and
    // queue_index will be populated in simulated_channel_.  Put this at the
    // back of the data segment.
    memcpy(message_->data(simulated_channel_->max_size()) +
               simulated_channel_->max_size() - size,
           msg, size);

    return DoSend(size, monotonic_remote_time, realtime_remote_time,
                  remote_queue_index);
  }

 private:
  SimulatedChannel *simulated_channel_;
  EventLoop *event_loop_;

  std::shared_ptr<SimulatedMessage> message_;
};
}  // namespace

class SimulatedFetcher : public RawFetcher {
 public:
  explicit SimulatedFetcher(EventLoop *event_loop,
                            SimulatedChannel *simulated_channel)
      : RawFetcher(event_loop, simulated_channel->channel()),
        simulated_channel_(simulated_channel) {}
  ~SimulatedFetcher() { simulated_channel_->UnregisterFetcher(this); }

  std::pair<bool, monotonic_clock::time_point> DoFetchNext() override {
    if (msgs_.size() == 0) {
      return std::make_pair(false, monotonic_clock::min_time);
    }

    SetMsg(msgs_.front());
    msgs_.pop_front();
    return std::make_pair(true, event_loop()->monotonic_now());
  }

  std::pair<bool, monotonic_clock::time_point> DoFetch() override {
    if (msgs_.size() == 0) {
      // TODO(austin): Can we just do this logic unconditionally?  It is a lot
      // simpler.  And call clear, obviously.
      if (!msg_ && simulated_channel_->latest_message()) {
        SetMsg(simulated_channel_->latest_message());
        return std::make_pair(true, event_loop()->monotonic_now());
      } else {
        return std::make_pair(false, monotonic_clock::min_time);
      }
    }

    // We've had a message enqueued, so we don't need to go looking for the
    // latest message from before we started.
    SetMsg(msgs_.back());
    msgs_.clear();
    return std::make_pair(true, event_loop()->monotonic_now());
  }

 private:
  friend class SimulatedChannel;

  // Updates the state inside RawFetcher to point to the data in msg_.
  void SetMsg(std::shared_ptr<SimulatedMessage> msg) {
    msg_ = msg;
    context_ = msg_->context;
    if (context_.remote_queue_index == 0xffffffffu) {
      context_.remote_queue_index = context_.queue_index;
    }
    if (context_.monotonic_remote_time == aos::monotonic_clock::min_time) {
      context_.monotonic_remote_time = context_.monotonic_event_time;
    }
    if (context_.realtime_remote_time == aos::realtime_clock::min_time) {
      context_.realtime_remote_time = context_.realtime_event_time;
    }
  }

  // Internal method for Simulation to add a message to the buffer.
  void Enqueue(std::shared_ptr<SimulatedMessage> buffer) {
    msgs_.emplace_back(buffer);
  }

  SimulatedChannel *simulated_channel_;
  std::shared_ptr<SimulatedMessage> msg_;

  // Messages queued up but not in use.
  ::std::deque<std::shared_ptr<SimulatedMessage>> msgs_;
};

class SimulatedTimerHandler : public TimerHandler {
 public:
  explicit SimulatedTimerHandler(EventScheduler *scheduler,
                                 NodeEventLoopFactory *node_event_loop_factory,
                                 SimulatedEventLoop *simulated_event_loop,
                                 ::std::function<void()> fn);
  ~SimulatedTimerHandler() { Disable(); }

  void Setup(monotonic_clock::time_point base,
             monotonic_clock::duration repeat_offset) override;

  void HandleEvent();

  void Disable() override;

 private:
  SimulatedEventLoop *simulated_event_loop_;
  EventHandler<SimulatedTimerHandler> event_;
  EventScheduler *scheduler_;
  NodeEventLoopFactory *node_event_loop_factory_;
  EventScheduler::Token token_;

  monotonic_clock::time_point base_;
  monotonic_clock::duration repeat_offset_;
};

class SimulatedPhasedLoopHandler : public PhasedLoopHandler {
 public:
  SimulatedPhasedLoopHandler(EventScheduler *scheduler,
                             NodeEventLoopFactory *node_event_loop_factory,
                             SimulatedEventLoop *simulated_event_loop,
                             ::std::function<void(int)> fn,
                             const monotonic_clock::duration interval,
                             const monotonic_clock::duration offset);
  ~SimulatedPhasedLoopHandler();

  void HandleEvent();

  void Schedule(monotonic_clock::time_point sleep_time) override;

 private:
  SimulatedEventLoop *simulated_event_loop_;
  EventHandler<SimulatedPhasedLoopHandler> event_;

  EventScheduler *scheduler_;
  NodeEventLoopFactory *node_event_loop_factory_;
  EventScheduler::Token token_;
};

class SimulatedEventLoop : public EventLoop {
 public:
  explicit SimulatedEventLoop(
      EventScheduler *scheduler,
      NodeEventLoopFactory *node_event_loop_factory,
      absl::btree_map<SimpleChannel, std::unique_ptr<SimulatedChannel>>
          *channels,
      const Configuration *configuration,
      std::vector<std::pair<EventLoop *, std::function<void(bool)>>>
          *raw_event_loops,
      const Node *node, pid_t tid)
      : EventLoop(CHECK_NOTNULL(configuration)),
        scheduler_(scheduler),
        node_event_loop_factory_(node_event_loop_factory),
        channels_(channels),
        raw_event_loops_(raw_event_loops),
        node_(node),
        tid_(tid) {
    raw_event_loops_->push_back(std::make_pair(this, [this](bool value) {
      if (!has_setup_) {
        Setup();
        has_setup_ = true;
      }
      set_is_running(value);
    }));
  }
  ~SimulatedEventLoop() override {
    // Trigger any remaining senders or fetchers to be cleared before destroying
    // the event loop so the book keeping matches.
    timing_report_sender_.reset();

    // Force everything with a registered fd with epoll to be destroyed now.
    timers_.clear();
    phased_loops_.clear();
    watchers_.clear();

    for (auto it = raw_event_loops_->begin(); it != raw_event_loops_->end();
         ++it) {
      if (it->first == this) {
        raw_event_loops_->erase(it);
        break;
      }
    }
  }

  std::chrono::nanoseconds send_delay() const { return send_delay_; }
  void set_send_delay(std::chrono::nanoseconds send_delay) {
    send_delay_ = send_delay;
  }

  ::aos::monotonic_clock::time_point monotonic_now() override {
    return node_event_loop_factory_->monotonic_now();
  }

  ::aos::realtime_clock::time_point realtime_now() override {
    return node_event_loop_factory_->realtime_now();
  }

  ::std::unique_ptr<RawSender> MakeRawSender(const Channel *channel) override;

  ::std::unique_ptr<RawFetcher> MakeRawFetcher(const Channel *channel) override;

  void MakeRawWatcher(
      const Channel *channel,
      ::std::function<void(const Context &context, const void *message)>
          watcher) override;

  TimerHandler *AddTimer(::std::function<void()> callback) override {
    CHECK(!is_running());
    return NewTimer(::std::unique_ptr<TimerHandler>(new SimulatedTimerHandler(
        scheduler_, node_event_loop_factory_, this, callback)));
  }

  PhasedLoopHandler *AddPhasedLoop(::std::function<void(int)> callback,
                                   const monotonic_clock::duration interval,
                                   const monotonic_clock::duration offset =
                                       ::std::chrono::seconds(0)) override {
    return NewPhasedLoop(::std::unique_ptr<PhasedLoopHandler>(
        new SimulatedPhasedLoopHandler(scheduler_, node_event_loop_factory_,
                                       this, callback, interval, offset)));
  }

  void OnRun(::std::function<void()> on_run) override {
    scheduler_->ScheduleOnRun(on_run);
  }

  const Node *node() const override { return node_; }

  void set_name(const std::string_view name) override {
    name_ = std::string(name);
  }
  const std::string_view name() const override { return name_; }

  SimulatedChannel *GetSimulatedChannel(const Channel *channel);

  void SetRuntimeRealtimePriority(int priority) override {
    CHECK(!is_running()) << ": Cannot set realtime priority while running.";
    priority_ = priority;
  }

  int priority() const override { return priority_; }

  void Setup() { MaybeScheduleTimingReports(); }

 private:
  friend class SimulatedTimerHandler;
  friend class SimulatedPhasedLoopHandler;
  friend class SimulatedWatcher;

  void HandleEvent() {
    while (true) {
      if (EventCount() == 0 || PeekEvent()->event_time() > monotonic_now()) {
        break;
      }

      EventLoopEvent *event = PopEvent();
      event->HandleEvent();
    }
  }

  pid_t GetTid() override { return tid_; }

  EventScheduler *scheduler_;
  NodeEventLoopFactory *node_event_loop_factory_;
  absl::btree_map<SimpleChannel, std::unique_ptr<SimulatedChannel>> *channels_;
  std::vector<std::pair<EventLoop *, std::function<void(bool)>>>
      *raw_event_loops_;

  ::std::string name_;

  int priority_ = 0;

  bool has_setup_ = false;

  std::chrono::nanoseconds send_delay_;

  const Node *const node_;
  const pid_t tid_;
};

void SimulatedEventLoopFactory::set_send_delay(
    std::chrono::nanoseconds send_delay) {
  send_delay_ = send_delay;
  for (std::pair<EventLoop *, std::function<void(bool)>> &loop :
       raw_event_loops_) {
    reinterpret_cast<SimulatedEventLoop *>(loop.first)
        ->set_send_delay(send_delay_);
  }
}

void SimulatedEventLoop::MakeRawWatcher(
    const Channel *channel,
    std::function<void(const Context &channel, const void *message)> watcher) {
  TakeWatcher(channel);

  std::unique_ptr<SimulatedWatcher> shm_watcher(new SimulatedWatcher(
      this, scheduler_, node_event_loop_factory_, channel, std::move(watcher)));

  GetSimulatedChannel(channel)->MakeRawWatcher(shm_watcher.get());
  NewWatcher(std::move(shm_watcher));
}

std::unique_ptr<RawSender> SimulatedEventLoop::MakeRawSender(
    const Channel *channel) {
  TakeSender(channel);

  return GetSimulatedChannel(channel)->MakeRawSender(this);
}

std::unique_ptr<RawFetcher> SimulatedEventLoop::MakeRawFetcher(
    const Channel *channel) {
  ChannelIndex(channel);

  if (!configuration::ChannelIsReadableOnNode(channel, node())) {
    LOG(FATAL) << "Channel { \"name\": \"" << channel->name()->string_view()
               << "\", \"type\": \"" << channel->type()->string_view()
               << "\" } is not able to be fetched on this node.  Check your "
                  "configuration.";
  }

  return GetSimulatedChannel(channel)->MakeRawFetcher(this);
}

SimulatedChannel *SimulatedEventLoop::GetSimulatedChannel(
    const Channel *channel) {
  auto it = channels_->find(SimpleChannel(channel));
  if (it == channels_->end()) {
    it = channels_
             ->emplace(SimpleChannel(channel),
                       std::unique_ptr<SimulatedChannel>(
                           new SimulatedChannel(channel, scheduler_)))
             .first;
  }
  return it->second.get();
}

SimulatedWatcher::SimulatedWatcher(
    SimulatedEventLoop *simulated_event_loop, EventScheduler *scheduler,
    NodeEventLoopFactory *node_event_loop_factory, const Channel *channel,
    std::function<void(const Context &context, const void *message)> fn)
    : WatcherState(simulated_event_loop, channel, std::move(fn)),
      simulated_event_loop_(simulated_event_loop),
      event_(this),
      scheduler_(scheduler),
      node_event_loop_factory_(node_event_loop_factory),
      token_(scheduler_->InvalidToken()) {}

SimulatedWatcher::~SimulatedWatcher() {
  simulated_event_loop_->RemoveEvent(&event_);
  if (token_ != scheduler_->InvalidToken()) {
    scheduler_->Deschedule(token_);
  }
  simulated_channel_->RemoveWatcher(this);
}

void SimulatedWatcher::Schedule(std::shared_ptr<SimulatedMessage> message) {
  monotonic_clock::time_point event_time =
      simulated_event_loop_->monotonic_now();

  // Messages are queued in order.  If we are the first, add ourselves.
  // Otherwise, don't.
  if (msgs_.size() == 0) {
    event_.set_event_time(message->context.monotonic_event_time);
    simulated_event_loop_->AddEvent(&event_);

    DoSchedule(event_time);
  }

  msgs_.emplace_back(message);
}

void SimulatedWatcher::HandleEvent() {
  CHECK_NE(msgs_.size(), 0u) << ": No events to handle.";

  const monotonic_clock::time_point monotonic_now =
      simulated_event_loop_->monotonic_now();
  Context context = msgs_.front()->context;

  if (context.remote_queue_index == 0xffffffffu) {
    context.remote_queue_index = context.queue_index;
  }
  if (context.monotonic_remote_time == aos::monotonic_clock::min_time) {
    context.monotonic_remote_time = context.monotonic_event_time;
  }
  if (context.realtime_remote_time == aos::realtime_clock::min_time) {
    context.realtime_remote_time = context.realtime_event_time;
  }

  DoCallCallback([monotonic_now]() { return monotonic_now; }, context);

  msgs_.pop_front();
  if (msgs_.size() != 0) {
    event_.set_event_time(msgs_.front()->context.monotonic_event_time);
    simulated_event_loop_->AddEvent(&event_);

    DoSchedule(event_.event_time());
  } else {
    token_ = scheduler_->InvalidToken();
  }
}

void SimulatedWatcher::DoSchedule(monotonic_clock::time_point event_time) {
  token_ = scheduler_->Schedule(
      node_event_loop_factory_->ToDistributedClock(
          event_time + simulated_event_loop_->send_delay()),
      [this]() { simulated_event_loop_->HandleEvent(); });
}

void SimulatedChannel::MakeRawWatcher(SimulatedWatcher *watcher) {
  watcher->SetSimulatedChannel(this);
  watchers_.emplace_back(watcher);
}

::std::unique_ptr<RawSender> SimulatedChannel::MakeRawSender(
    EventLoop *event_loop) {
  return ::std::unique_ptr<RawSender>(new SimulatedSender(this, event_loop));
}

::std::unique_ptr<RawFetcher> SimulatedChannel::MakeRawFetcher(
    EventLoop *event_loop) {
  ::std::unique_ptr<SimulatedFetcher> fetcher(
      new SimulatedFetcher(event_loop, this));
  fetchers_.push_back(fetcher.get());
  return ::std::move(fetcher);
}

uint32_t SimulatedChannel::Send(std::shared_ptr<SimulatedMessage> message) {
  const uint32_t queue_index = next_queue_index_.index();
  message->context.queue_index = queue_index;
  message->context.data = message->data(channel()->max_size()) +
                          channel()->max_size() - message->context.size;
  next_queue_index_ = next_queue_index_.Increment();

  latest_message_ = message;
  if (scheduler_->is_running()) {
    for (SimulatedWatcher *watcher : watchers_) {
      watcher->Schedule(message);
    }
  }
  for (auto &fetcher : fetchers_) {
    fetcher->Enqueue(message);
  }

  return queue_index;
}

void SimulatedChannel::UnregisterFetcher(SimulatedFetcher *fetcher) {
  fetchers_.erase(::std::find(fetchers_.begin(), fetchers_.end(), fetcher));
}

SimulatedTimerHandler::SimulatedTimerHandler(
    EventScheduler *scheduler, NodeEventLoopFactory *node_event_loop_factory,
    SimulatedEventLoop *simulated_event_loop, ::std::function<void()> fn)
    : TimerHandler(simulated_event_loop, std::move(fn)),
      simulated_event_loop_(simulated_event_loop),
      event_(this),
      scheduler_(scheduler),
      node_event_loop_factory_(node_event_loop_factory),
      token_(scheduler_->InvalidToken()) {}

void SimulatedTimerHandler::Setup(monotonic_clock::time_point base,
                                  monotonic_clock::duration repeat_offset) {
  Disable();
  const ::aos::monotonic_clock::time_point monotonic_now =
      simulated_event_loop_->monotonic_now();
  base_ = base;
  repeat_offset_ = repeat_offset;
  if (base < monotonic_now) {
    token_ = scheduler_->Schedule(
        node_event_loop_factory_->ToDistributedClock(monotonic_now),
        [this]() { simulated_event_loop_->HandleEvent(); });
  } else {
    token_ = scheduler_->Schedule(
        node_event_loop_factory_->ToDistributedClock(base),
        [this]() { simulated_event_loop_->HandleEvent(); });
  }
  event_.set_event_time(base_);
  simulated_event_loop_->AddEvent(&event_);
}

void SimulatedTimerHandler::HandleEvent() {
  const ::aos::monotonic_clock::time_point monotonic_now =
      simulated_event_loop_->monotonic_now();
  if (repeat_offset_ != ::aos::monotonic_clock::zero()) {
    // Reschedule.
    while (base_ <= monotonic_now) base_ += repeat_offset_;
    token_ = scheduler_->Schedule(
        node_event_loop_factory_->ToDistributedClock(base_),
        [this]() { simulated_event_loop_->HandleEvent(); });
    event_.set_event_time(base_);
    simulated_event_loop_->AddEvent(&event_);
  } else {
    token_ = scheduler_->InvalidToken();
  }

  Call([monotonic_now]() { return monotonic_now; }, monotonic_now);
}

void SimulatedTimerHandler::Disable() {
  simulated_event_loop_->RemoveEvent(&event_);
  if (token_ != scheduler_->InvalidToken()) {
    scheduler_->Deschedule(token_);
    token_ = scheduler_->InvalidToken();
  }
}

SimulatedPhasedLoopHandler::SimulatedPhasedLoopHandler(
    EventScheduler *scheduler, NodeEventLoopFactory *node_event_loop_factory,
    SimulatedEventLoop *simulated_event_loop, ::std::function<void(int)> fn,
    const monotonic_clock::duration interval,
    const monotonic_clock::duration offset)
    : PhasedLoopHandler(simulated_event_loop, std::move(fn), interval, offset),
      simulated_event_loop_(simulated_event_loop),
      event_(this),
      scheduler_(scheduler),
      node_event_loop_factory_(node_event_loop_factory),
      token_(scheduler_->InvalidToken()) {}

SimulatedPhasedLoopHandler::~SimulatedPhasedLoopHandler() {
  if (token_ != scheduler_->InvalidToken()) {
    scheduler_->Deschedule(token_);
    token_ = scheduler_->InvalidToken();
  }
  simulated_event_loop_->RemoveEvent(&event_);
}

void SimulatedPhasedLoopHandler::HandleEvent() {
  monotonic_clock::time_point monotonic_now =
      simulated_event_loop_->monotonic_now();
  Call(
      [monotonic_now]() { return monotonic_now; },
      [this](monotonic_clock::time_point sleep_time) { Schedule(sleep_time); });
}

void SimulatedPhasedLoopHandler::Schedule(
    monotonic_clock::time_point sleep_time) {
  token_ = scheduler_->Schedule(
      node_event_loop_factory_->ToDistributedClock(sleep_time),
      [this]() { simulated_event_loop_->HandleEvent(); });
  event_.set_event_time(sleep_time);
  simulated_event_loop_->AddEvent(&event_);
}

NodeEventLoopFactory::NodeEventLoopFactory(
    EventScheduler *scheduler, SimulatedEventLoopFactory *factory,
    const Node *node,
    std::vector<std::pair<EventLoop *, std::function<void(bool)>>>
        *raw_event_loops)
    : scheduler_(scheduler),
      factory_(factory),
      node_(node),
      raw_event_loops_(raw_event_loops) {}

SimulatedEventLoopFactory::SimulatedEventLoopFactory(
    const Configuration *configuration)
    : configuration_(CHECK_NOTNULL(configuration)) {
  if (configuration::MultiNode(configuration_)) {
    for (const Node *node : *configuration->nodes()) {
      nodes_.emplace_back(node);
    }
  } else {
    nodes_.emplace_back(nullptr);
  }

  for (const Node *node : nodes_) {
    node_factories_.emplace_back(
        new NodeEventLoopFactory(&scheduler_, this, node, &raw_event_loops_));
  }

  if (configuration::MultiNode(configuration)) {
    bridge_ = std::make_unique<message_bridge::SimulatedMessageBridge>(this);
  }
}

SimulatedEventLoopFactory::~SimulatedEventLoopFactory() {}

NodeEventLoopFactory *SimulatedEventLoopFactory::GetNodeEventLoopFactory(
    const Node *node) {
  auto result = std::find_if(
      node_factories_.begin(), node_factories_.end(),
      [node](const std::unique_ptr<NodeEventLoopFactory> &node_factory) {
        return node_factory->node() == node;
      });

  CHECK(result != node_factories_.end())
      << ": Failed to find node " << FlatbufferToJson(node);

  return result->get();
}

::std::unique_ptr<EventLoop> SimulatedEventLoopFactory::MakeEventLoop(
    std::string_view name, const Node *node) {
  if (node == nullptr) {
    CHECK(!configuration::MultiNode(configuration()))
        << ": Can't make a single node event loop in a multi-node world.";
  } else {
    CHECK(configuration::MultiNode(configuration()))
        << ": Can't make a multi-node event loop in a single-node world.";
  }
  return GetNodeEventLoopFactory(node)->MakeEventLoop(name);
}

::std::unique_ptr<EventLoop> NodeEventLoopFactory::MakeEventLoop(
    std::string_view name) {
  pid_t tid = tid_;
  ++tid_;
  ::std::unique_ptr<SimulatedEventLoop> result(new SimulatedEventLoop(
      scheduler_, this, &channels_, factory_->configuration(), raw_event_loops_,
      node_, tid));
  result->set_name(name);
  result->set_send_delay(factory_->send_delay());
  return std::move(result);
}

void SimulatedEventLoopFactory::RunFor(monotonic_clock::duration duration) {
  for (const std::pair<EventLoop *, std::function<void(bool)>> &event_loop :
       raw_event_loops_) {
    event_loop.second(true);
  }
  scheduler_.RunFor(duration);
  for (const std::pair<EventLoop *, std::function<void(bool)>> &event_loop :
       raw_event_loops_) {
    event_loop.second(false);
  }
}

void SimulatedEventLoopFactory::Run() {
  for (const std::pair<EventLoop *, std::function<void(bool)>> &event_loop :
       raw_event_loops_) {
    event_loop.second(true);
  }
  scheduler_.Run();
  for (const std::pair<EventLoop *, std::function<void(bool)>> &event_loop :
       raw_event_loops_) {
    event_loop.second(false);
  }
}

}  // namespace aos
