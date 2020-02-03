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
                                                                 name),
      hood_(constants::GetValues().hood) {
  event_loop->SetRuntimeRealtimePriority(30);
}

void Superstructure::RunIteration(const Goal *unsafe_goal,
                                  const Position *position,
                                  aos::Sender<Output>::Builder *output,
                                  aos::Sender<Status>::Builder *status) {
  if (WasReset()) {
    AOS_LOG(ERROR, "WPILib reset, restarting\n");
    hood_.Reset();
  }

  OutputT output_struct;

  flatbuffers::Offset<AbsoluteEncoderProfiledJointStatus> hood_status_offset =
      hood_.Iterate(unsafe_goal != nullptr ? unsafe_goal->hood() : nullptr,
                    position->hood(),
                    output != nullptr ? &(output_struct.hood_voltage) : nullptr,
                    status->fbb());

  bool zeroed;
  bool estopped;

  const AbsoluteEncoderProfiledJointStatus *hood_status =
      GetMutableTemporaryPointer(*status->fbb(), hood_status_offset);
  zeroed = hood_status->zeroed();
  estopped = hood_status->estopped();

  if (output != nullptr) {
    output->Send(Output::Pack(*output->fbb(), &output_struct));
  }

  Status::Builder status_builder = status->MakeBuilder<Status>();

  status_builder.add_zeroed(zeroed);
  status_builder.add_estopped(estopped);

  status_builder.add_hood(hood_status_offset);

  status->Send(status_builder.Finish());
}

}  // namespace superstructure
}  // namespace control_loops
}  // namespace y2020
