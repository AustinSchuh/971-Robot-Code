#include <unistd.h>

#include <chrono>
#include <memory>

#include "aos/controls/control_loop_test.h"
#include "frc971/control_loops/capped_test_plant.h"
#include "frc971/control_loops/position_sensor_sim.h"
#include "frc971/control_loops/team_number_test_environment.h"
#include "gtest/gtest.h"
#include "y2020/constants.h"
#include "y2020/control_loops/superstructure/hood/hood_plant.h"
#include "y2020/control_loops/superstructure/superstructure.h"

namespace y2020 {
namespace control_loops {
namespace superstructure {
namespace testing {

namespace {
constexpr double kNoiseScalar = 0.01;
}  // namespace

namespace chrono = ::std::chrono;
using ::aos::monotonic_clock;
using ::frc971::CreateProfileParameters;
using ::frc971::control_loops::CappedTestPlant;
using ::frc971::control_loops::
    CreateStaticZeroingSingleDOFProfiledSubsystemGoal;
using ::frc971::control_loops::PositionSensorSimulator;
using ::frc971::control_loops::StaticZeroingSingleDOFProfiledSubsystemGoal;
typedef Superstructure::AbsoluteEncoderSubsystem AbsoluteEncoderSubsystem;

// Class which simulates the superstructure and sends out queue messages with
// the position.
class SuperstructureSimulation {
 public:
  SuperstructureSimulation(::aos::EventLoop *event_loop, chrono::nanoseconds dt)
      : event_loop_(event_loop),
        dt_(dt),
        superstructure_position_sender_(
            event_loop_->MakeSender<Position>("/superstructure")),
        superstructure_status_fetcher_(
            event_loop_->MakeFetcher<Status>("/superstructure")),
        superstructure_output_fetcher_(
            event_loop_->MakeFetcher<Output>("/superstructure")),

        hood_plant_(new CappedTestPlant(hood::MakeHoodPlant())),
        hood_encoder_(constants::GetValues()
                          .hood.zeroing_constants.one_revolution_distance) {
    InitializeHoodPosition(constants::Values::kHoodRange().upper);

    phased_loop_handle_ = event_loop_->AddPhasedLoop(
        [this](int) {
          // Skip this the first time.
          if (!first_) {
            Simulate();
          }
          first_ = false;
          SendPositionMessage();
        },
        dt);
  }

  void InitializeHoodPosition(double start_pos) {
    hood_plant_->mutable_X(0, 0) = start_pos;
    hood_plant_->mutable_X(1, 0) = 0.0;

    hood_encoder_.Initialize(
        start_pos, kNoiseScalar, 0.0,
        constants::GetValues()
            .hood.zeroing_constants.measured_absolute_position);
  }

  // Sends a queue message with the position of the superstructure.
  void SendPositionMessage() {
    ::aos::Sender<Position>::Builder builder =
        superstructure_position_sender_.MakeBuilder();

    frc971::AbsolutePosition::Builder hood_builder =
        builder.MakeBuilder<frc971::AbsolutePosition>();
    flatbuffers::Offset<frc971::AbsolutePosition> hood_offset =
        hood_encoder_.GetSensorValues(&hood_builder);

    Position::Builder position_builder = builder.MakeBuilder<Position>();

    position_builder.add_hood(hood_offset);

    builder.Send(position_builder.Finish());
  }

  double hood_position() const { return hood_plant_->X(0, 0); }
  double hood_velocity() const { return hood_plant_->X(1, 0); }

  // Simulates the superstructure for a single timestep.
  void Simulate() {
    const double last_hood_velocity = hood_velocity();

    EXPECT_TRUE(superstructure_output_fetcher_.Fetch());
    EXPECT_TRUE(superstructure_status_fetcher_.Fetch());

    const double voltage_check_hood =
        (static_cast<AbsoluteEncoderSubsystem::State>(
             superstructure_status_fetcher_->hood()->state()) ==
         AbsoluteEncoderSubsystem::State::RUNNING)
            ? constants::GetValues().hood.operating_voltage
            : constants::GetValues().hood.zeroing_voltage;

    EXPECT_NEAR(superstructure_output_fetcher_->hood_voltage(), 0.0,
                voltage_check_hood);

    ::Eigen::Matrix<double, 1, 1> hood_U;
    hood_U << superstructure_output_fetcher_->hood_voltage() +
                  hood_plant_->voltage_offset();

    hood_plant_->Update(hood_U);

    const double position_hood = hood_plant_->Y(0, 0);

    hood_encoder_.MoveTo(position_hood);

    EXPECT_GE(position_hood, constants::Values::kHoodRange().lower_hard);
    EXPECT_LE(position_hood, constants::Values::kHoodRange().upper_hard);

    const double loop_time = ::aos::time::DurationInSeconds(dt_);

    const double hood_acceleration =
        (hood_velocity() - last_hood_velocity) / loop_time;

    EXPECT_GE(peak_hood_acceleration_, hood_acceleration);
    EXPECT_LE(-peak_hood_acceleration_, hood_acceleration);
    EXPECT_GE(peak_hood_velocity_, hood_velocity());
    EXPECT_LE(-peak_hood_velocity_, hood_velocity());
  }

  void set_peak_hood_acceleration(double value) {
    peak_hood_acceleration_ = value;
  }
  void set_peak_hood_velocity(double value) { peak_hood_velocity_ = value; }

 private:
  ::aos::EventLoop *event_loop_;
  const chrono::nanoseconds dt_;
  ::aos::PhasedLoopHandler *phased_loop_handle_ = nullptr;

  ::aos::Sender<Position> superstructure_position_sender_;
  ::aos::Fetcher<Status> superstructure_status_fetcher_;
  ::aos::Fetcher<Output> superstructure_output_fetcher_;

  bool first_ = true;

  ::std::unique_ptr<CappedTestPlant> hood_plant_;
  PositionSensorSimulator hood_encoder_;

  // The acceleration limits to check for while moving.
  double peak_hood_acceleration_ = 1e10;

  // The velocity limits to check for while moving.
  double peak_hood_velocity_ = 1e10;
};

class SuperstructureTest : public ::aos::testing::ControlLoopTest {
 protected:
  SuperstructureTest()
      : ::aos::testing::ControlLoopTest(
            aos::configuration::ReadConfig("y2020/config.json"),
            chrono::microseconds(5050)),
        test_event_loop_(MakeEventLoop("test")),
        superstructure_goal_fetcher_(
            test_event_loop_->MakeFetcher<Goal>("/superstructure")),
        superstructure_goal_sender_(
            test_event_loop_->MakeSender<Goal>("/superstructure")),
        superstructure_status_fetcher_(
            test_event_loop_->MakeFetcher<Status>("/superstructure")),
        superstructure_output_fetcher_(
            test_event_loop_->MakeFetcher<Output>("/superstructure")),
        superstructure_position_fetcher_(
            test_event_loop_->MakeFetcher<Position>("/superstructure")),
        superstructure_event_loop_(MakeEventLoop("superstructure")),
        superstructure_(superstructure_event_loop_.get()),
        superstructure_plant_event_loop_(MakeEventLoop("plant")),
        superstructure_plant_(superstructure_plant_event_loop_.get(), dt()) {
    set_team_id(::frc971::control_loops::testing::kTeamNumber);
  }

  void VerifyNearGoal() {
    superstructure_goal_fetcher_.Fetch();
    superstructure_status_fetcher_.Fetch();

    EXPECT_NEAR(superstructure_goal_fetcher_->hood()->unsafe_goal(),
                superstructure_status_fetcher_->hood()->position(), 0.001);
  }

  void CheckIfZeroed() {
      superstructure_status_fetcher_.Fetch();
      ASSERT_TRUE(superstructure_status_fetcher_.get()->zeroed());
  }

  void WaitUntilZeroed() {
    int i = 0;
    do {
      i++;
      RunFor(dt());
      superstructure_status_fetcher_.Fetch();
      // 2 Seconds
      ASSERT_LE(i, 2.0 / ::aos::time::DurationInSeconds(dt()));

      // Since there is a delay when sending running, make sure we have a status
      // before checking it.
    } while (superstructure_status_fetcher_.get() == nullptr ||
             !superstructure_status_fetcher_.get()->zeroed());
  }

  ::std::unique_ptr<::aos::EventLoop> test_event_loop_;

  ::aos::Fetcher<Goal> superstructure_goal_fetcher_;
  ::aos::Sender<Goal> superstructure_goal_sender_;
  ::aos::Fetcher<Status> superstructure_status_fetcher_;
  ::aos::Fetcher<Output> superstructure_output_fetcher_;
  ::aos::Fetcher<Position> superstructure_position_fetcher_;

  // Create a control loop and simulation.
  ::std::unique_ptr<::aos::EventLoop> superstructure_event_loop_;
  Superstructure superstructure_;

  ::std::unique_ptr<::aos::EventLoop> superstructure_plant_event_loop_;
  SuperstructureSimulation superstructure_plant_;
};

// Tests that the superstructure does nothing when the goal is to remain still.
TEST_F(SuperstructureTest, DoesNothing) {
  SetEnabled(true);
  superstructure_plant_.InitializeHoodPosition(0.77);

  WaitUntilZeroed();

  {
    auto builder = superstructure_goal_sender_.MakeBuilder();

    flatbuffers::Offset<StaticZeroingSingleDOFProfiledSubsystemGoal>
        hood_offset = CreateStaticZeroingSingleDOFProfiledSubsystemGoal(
            *builder.fbb(), 0.77);

    Goal::Builder goal_builder = builder.MakeBuilder<Goal>();

    goal_builder.add_hood(hood_offset);

    ASSERT_TRUE(builder.Send(goal_builder.Finish()));
  }
  RunFor(chrono::seconds(10));
  VerifyNearGoal();

  EXPECT_TRUE(superstructure_output_fetcher_.Fetch());
}

// Tests that loops can reach a goal.
TEST_F(SuperstructureTest, ReachesGoal) {
  SetEnabled(true);
  // Set a reasonable goal.

  superstructure_plant_.InitializeHoodPosition(0.7);

  WaitUntilZeroed();
  {
    auto builder = superstructure_goal_sender_.MakeBuilder();

    flatbuffers::Offset<StaticZeroingSingleDOFProfiledSubsystemGoal>
        hood_offset = CreateStaticZeroingSingleDOFProfiledSubsystemGoal(
            *builder.fbb(), 0.2,
            CreateProfileParameters(*builder.fbb(), 1.0, 0.2));

    Goal::Builder goal_builder = builder.MakeBuilder<Goal>();

    goal_builder.add_hood(hood_offset);

    ASSERT_TRUE(builder.Send(goal_builder.Finish()));
  }

  // Give it a lot of time to get there.
  RunFor(chrono::seconds(8));

  VerifyNearGoal();
}
// Makes sure that the voltage on a motor is properly pulled back after
// saturation such that we don't get weird or bad (e.g. oscillating) behaviour.
//
// We are going to disable collision detection to make this easier to implement.
TEST_F(SuperstructureTest, SaturationTest) {
  SetEnabled(true);
  // Zero it before we move.
  WaitUntilZeroed();
  {
    auto builder = superstructure_goal_sender_.MakeBuilder();

    flatbuffers::Offset<StaticZeroingSingleDOFProfiledSubsystemGoal>
        hood_offset = CreateStaticZeroingSingleDOFProfiledSubsystemGoal(
            *builder.fbb(), constants::Values::kHoodRange().upper);

    Goal::Builder goal_builder = builder.MakeBuilder<Goal>();

    goal_builder.add_hood(hood_offset);

    ASSERT_TRUE(builder.Send(goal_builder.Finish()));
  }
  RunFor(chrono::seconds(8));
  VerifyNearGoal();

  // Try a low acceleration move with a high max velocity and verify the
  // acceleration is capped like expected.
  {
    auto builder = superstructure_goal_sender_.MakeBuilder();

    flatbuffers::Offset<StaticZeroingSingleDOFProfiledSubsystemGoal>
        hood_offset = CreateStaticZeroingSingleDOFProfiledSubsystemGoal(
            *builder.fbb(), constants::Values::kHoodRange().lower,
            CreateProfileParameters(*builder.fbb(), 20.0, 0.1));

    Goal::Builder goal_builder = builder.MakeBuilder<Goal>();

    goal_builder.add_hood(hood_offset);

    ASSERT_TRUE(builder.Send(goal_builder.Finish()));
  }
  superstructure_plant_.set_peak_hood_velocity(23.0);
  superstructure_plant_.set_peak_hood_acceleration(0.2);

  RunFor(chrono::seconds(8));
  VerifyNearGoal();
}

// Tests that the loop zeroes when run for a while without a goal.
TEST_F(SuperstructureTest, ZeroNoGoal) {
  SetEnabled(true);
  WaitUntilZeroed();
  RunFor(chrono::seconds(2));
  EXPECT_EQ(AbsoluteEncoderSubsystem::State::RUNNING,
            superstructure_.hood().state());
}

// Tests that running disabled works
TEST_F(SuperstructureTest, DisableTest) {
  RunFor(chrono::seconds(2));
  CheckIfZeroed();
}

}  // namespace testing
}  // namespace superstructure
}  // namespace control_loops
}  // namespace y2020
