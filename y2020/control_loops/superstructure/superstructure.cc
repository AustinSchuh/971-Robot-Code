#include "y2020/control_loops/superstructure/superstructure.h"

#include "aos/events/event_loop.h"

namespace y2020 {
namespace control_loops {
namespace superstructure {

using frc971::control_loops::AbsoluteEncoderProfiledJointStatus;
using frc971::control_loops::PotAndAbsoluteEncoderProfiledJointStatus;

Superstructure::Superstructure(::aos::EventLoop *event_loop,
                               const ::std::string &name)
    : aos::controls::ControlLoop<Goal, Position, Status, Output>(event_loop,
                                                                 name) {
      event_loop->SetRuntimeRealtimePriority(30);
}

void Superstructure::RunIteration(const Goal * /*unsafe_goal*/,
                                  const Position * /*position*/,
                                  aos::Sender<Output>::Builder *output,
                                  aos::Sender<Status>::Builder *status) {
  if (WasReset()) {
    AOS_LOG(ERROR, "WPILib reset, restarting\n");
  }


  if (output != nullptr) {
    OutputT output_struct;
    output->Send(Output::Pack(*output->fbb(), &output_struct));
  }

  Status::Builder status_builder = status->MakeBuilder<Status>();

  status_builder.add_zeroed(true);
  status_builder.add_estopped(false);

  status->Send(status_builder.Finish());
}

}  // namespace control_loops
}  // namespace y2020
}  // namespace y2020
