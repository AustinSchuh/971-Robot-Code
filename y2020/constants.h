#ifndef y2020_CONSTANTS_H_
#define y2020_CONSTANTS_H_

#include <math.h>
#include <stdint.h>

#include <array>

#include "frc971/constants.h"
#include "frc971/control_loops/pose.h"
#include "frc971/control_loops/static_zeroing_single_dof_profiled_subsystem.h"
#include "y2020/control_loops/drivetrain/drivetrain_dog_motor_plant.h"
#include "y2020/control_loops/superstructure/hood/hood_plant.h"

namespace y2020 {
namespace constants {

struct Values {
  static const int kZeroingSampleSize = 200;

  static constexpr double kDrivetrainCyclesPerRevolution() { return 512.0; }
  static constexpr double kDrivetrainEncoderCountsPerRevolution() {
    return kDrivetrainCyclesPerRevolution() * 4;
  }
  static constexpr double kDrivetrainEncoderRatio() { return (24.0 / 52.0); }
  static constexpr double kMaxDrivetrainEncoderPulsesPerSecond() {
    return control_loops::drivetrain::kFreeSpeed / (2.0 * M_PI) *
           control_loops::drivetrain::kHighOutputRatio /
           constants::Values::kDrivetrainEncoderRatio() *
           kDrivetrainEncoderCountsPerRevolution();
  }

  // Hood
  static constexpr double kHoodEncoderCountsPerRevolution() { return 4096.0; }

  //TODO(sabina): Update constants
  static constexpr double kHoodEncoderRatio() { return 1.0; }

  static constexpr double kMaxHoodEncoderPulsesPerSecond() {
    return control_loops::superstructure::hood::kFreeSpeed *
           control_loops::superstructure::hood::kOutputRatio /
           kHoodEncoderRatio() / (2.0 * M_PI) *
           kHoodEncoderCountsPerRevolution();
  }

  static constexpr ::frc971::constants::Range kHoodRange() {
    return ::frc971::constants::Range{
        0.00,  // Back Hard
        0.79,  // Front Hard
        0.14,  // Back Soft
        0.78   // Front Soft
    };
  }

  ::frc971::control_loops::StaticZeroingSingleDOFProfiledSubsystemParams<
      ::frc971::zeroing::AbsoluteEncoderZeroingEstimator>
      hood;
};

// Creates (once) a Values instance for ::aos::network::GetTeamNumber() and
// returns a reference to it.
const Values &GetValues();

// Creates Values instances for each team number it is called with and returns
// them.
const Values &GetValuesForTeam(uint16_t team_number);

}  // namespace constants
}  // namespace y2020

#endif  // y2020_CONSTANTS_H_
