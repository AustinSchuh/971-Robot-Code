#ifndef FRC971_CONTROL_LOOPS_POSITION_SENSOR_SIM_H_
#define FRC971_CONTROL_LOOPS_POSITION_SENSOR_SIM_H_

#include "aos/testing/random_seed.h"
#include "flatbuffers/flatbuffers.h"
#include "frc971/control_loops/control_loops_generated.h"
#include "frc971/control_loops/gaussian_noise.h"

namespace frc971 {
namespace control_loops {

// NOTE: All potentiometer and encoder values in this class are assumed to be in
// translated SI units.

class PositionSensorSimulator {
 public:
  // distance_per_revolution:
  //     The interval between index pulses. This is measured in SI units. For
  //     example, if an index pulse hits every 5cm on the elevator, set this to
  //     0.05.
  //     NOTE: When retrieving the sensor values for a PotAndAbsolutePosition
  //     message this field represents the interval between when the absolute
  //     encoder reads 0.
  // single_turn_distance_per_revolution:
  //    This is only used when retrieving
  //    sensor values for an AbsoluteAndAbsolutePosition.
  //    The interval between when the single turn abosolute encoder reads 0.
  //    This will be the whole range of the single turn absolute encoder
  //    as it cannot turn more than once.
  // noise_seed:
  //     The seed to feed into the random number generator for the potentiometer
  //     values.
  PositionSensorSimulator(
      double distance_per_revolution,
      double single_turn_distance_per_revolution = 0.0,
      unsigned int noise_seed = ::aos::testing::RandomSeed());

  // Set new parameters for the sensors. This is useful for unit tests to change
  // the simulated sensors' behavior on the fly.
  // start_position: The position relative to absolute zero where the simulated
  //                 structure starts. For example, to simulate the elevator
  //                 starting at 40cm above absolute zero, set this to 0.4.
  // pot_noise_stddev: The pot noise is sampled from a gaussian distribution.
  //                   This specifies the standard deviation of that
  //                   distribution.
  // known_index_pos: The absolute position of an index pulse.
  // known_absolute_encoder_pos: The readout of the absolute encoder when the
  //                             robot's mechanism is at zero.
  // single_turn_known_absolute_encoder_pos: The readout of the single turn
  //                                         absolute encoder when the robot's
  //                                         mechanism is at zero.
  void Initialize(double start_position, double pot_noise_stddev,
                  double known_index_pos = 0.0,
                  double known_absolute_encoder_pos = 0.0,
                  double single_turn_known_absolute_encoder_pos = 0.0);

  // Initializes a sensor simulation which is pretending to be a hall effect +
  // encoder setup.  This is assuming that the hall effect sensor triggers once
  // per cycle, and a cycle is distance_per_revolution_ long;
  void InitializeHallEffectAndPosition(double start_position,
                                       double known_index_lower,
                                       double known_index_upper);

  // Initializes a sensor simulation with a single relative enoder, providing
  // position information relative to a starting position of 0. For use with
  // RelativePosition sensor values.
  void InitializeRelativeEncoder();

  // Simulate the structure moving to a new position. The new value is measured
  // relative to absolute zero. This will update the simulated sensors with new
  // readings.
  // new_position: The new position relative to absolute zero.
  void MoveTo(double new_position);

  // Get the current values of the simulated sensors.
  // values: The target structure will be populated with simulated sensor
  //         readings. The readings will be in SI units. For example the units
  //         can be given in radians, meters, etc.
  template <typename PositionBuilder>
  flatbuffers::Offset<typename PositionBuilder::Table> GetSensorValues(
      PositionBuilder *builder);
  template <typename Position>
  const Position *FillSensorValues(flatbuffers::FlatBufferBuilder *fbb) {
    fbb->Clear();
    typename Position::Builder values(*fbb);

    fbb->Finish(GetSensorValues(&values));
    return flatbuffers::GetRoot<Position>(fbb->GetBufferPointer());
  }

 private:
  // It turns out that we can re-use a lot of the same logic to count the index
  // pulses as we can use to count the hall effect edges.  The trick is to add 2
  // "index pulses" spaced apart by the width of the hall effect sensor, and
  // call the sensor "high" when the index segments disagree for the two
  // IndexEdge classes disagree.
  class IndexEdge {
   public:
    explicit IndexEdge(double distance_per_revolution, unsigned int noise_seed)
        : distance_per_revolution_(distance_per_revolution),
          pot_noise_(noise_seed, 0.0) {}

    void Initialize(double start_position, double segment_position) {
      current_index_segment_ = ::std::floor(
          (start_position - segment_position) / distance_per_revolution_);
      known_index_position_ = segment_position;
      last_index_ = 0;
      index_count_ = 0;
    }

    int index_count() const { return index_count_; }
    double latched_pot() const { return latched_pot_; }
    int current_index_segment() const { return current_index_segment_; }

    double IndexPulsePosition() const {
      return last_index_ * distance_per_revolution_ + known_index_position_;
    }

    void MoveTo(double new_position) {
      const int new_index_segment = ::std::floor(
          (new_position - known_index_position_) / distance_per_revolution_);

      if (new_index_segment < current_index_segment_) {
        // We've crossed an index pulse in the negative direction. That means
        // the index pulse we just crossed is the higher end of the current
        // index segment. For example, if the mechanism moved from index segment
        // four to index segment three, then we just crossed index pulse 4.
        last_index_ = new_index_segment + 1;
        index_count_++;
      } else if (new_index_segment > current_index_segment_) {
        // We've crossed an index pulse in the positive direction. That means
        // the index pulse we just crossed is the lower end of the index
        // segment. For example, if the mechanism moved from index segment seven
        // to index segment eight, then we just crossed index pulse eight.
        last_index_ = new_index_segment;
        index_count_++;
      }

      if (new_index_segment != current_index_segment_) {
        latched_pot_ = pot_noise_.AddNoiseToSample(
            last_index_ * distance_per_revolution_ + known_index_position_);
      }

      current_index_segment_ = new_index_segment;
    }

    GaussianNoise *mutable_pot_noise() { return &pot_noise_; }

   private:
    // The absolute segment between two index pulses the simulation is on. For
    // example, when the current position is betwen index pulse zero and one,
    // the current index segment is considered to be zero. Index segment one is
    // between index pulses 1 and 2, etc.
    int current_index_segment_;
    // Index pulse to use for calculating latched sensor values, relative to
    // absolute zero. In other words this always holds the index pulse that was
    // encountered most recently.
    int last_index_;
    // How many index pulses we've seen.
    int index_count_;

    // Absolute position of a known index pulse.
    double known_index_position_;
    // Distance between index pulses on the mechanism.
    double distance_per_revolution_;

    // The pot position at the most recent index pulse with noise added.
    double latched_pot_;

    // Gaussian noise to add to pot readings.
    GaussianNoise pot_noise_;
  };

  double WrapAbsoluteEncoder() {
    // TODO(phil): Create some lag here since this is a PWM signal it won't be
    // instantaneous like the other signals. Better yet, its lag varies
    // randomly with the distribution varying depending on the reading.
    double absolute_encoder = ::std::remainder(
        current_position_ + known_absolute_encoder_, distance_per_revolution_);
    if (absolute_encoder < 0) {
      absolute_encoder += distance_per_revolution_;
    }
    return absolute_encoder;
  }

  IndexEdge lower_index_edge_;
  IndexEdge upper_index_edge_;

  // Distance between index pulses on the mechanism.
  double distance_per_revolution_;

  // The distance that the mechanism travels for one revolution of the single
  // turn absolue encoder
  double single_turn_distance_per_revolution_;

  // The readout of the absolute encoder when the robot's mechanism is at
  // zero.
  double known_absolute_encoder_;
  // The other absolute encoder's value when the mechanism is at zero.
  double single_turn_known_absolute_encoder_;
  // Current position of the mechanism relative to absolute zero.
  double current_position_;
  // Starting position of the mechanism relative to absolute zero. See the
  // `starting_position` parameter in the constructor for more info.
  double start_position_;

  int posedge_count_;
  int negedge_count_;
  double posedge_value_;
  double negedge_value_;
};

}  // namespace control_loops
}  // namespace frc971

#endif /* FRC971_CONTROL_LOOPS_POSITION_SENSOR_SIM_H_ */
