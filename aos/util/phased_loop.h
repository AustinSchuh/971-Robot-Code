#ifndef AOS_UTIL_PHASED_LOOP_H_
#define AOS_UTIL_PHASED_LOOP_H_

#include "aos/time/time.h"

namespace aos {
namespace time {

// Handles sleeping until a fixed offset from some time interval.
class PhasedLoop {
 public:
  // For example, with interval = 1s and offset = 0.1s this will fire at:
  //   0.1s
  //   1.1s
  //   ...
  //   10000.1s
  // offset must be >= chrono::seconds(0) and < interval.
  PhasedLoop(
      const monotonic_clock::duration interval,
      const monotonic_clock::time_point monotonic_now,
      const monotonic_clock::duration offset = monotonic_clock::duration(0));

  // Updates the offset and interval.
  void set_interval_and_offset(const monotonic_clock::duration interval,
                               const monotonic_clock::duration offset);

  // Computes the offset given an interval and a time that we should trigger.
  static monotonic_clock::duration OffsetFromIntervalAndTime(
      const monotonic_clock::duration interval,
      const monotonic_clock::time_point monotonic_trigger);

  // Resets the count of skipped iterations.
  // Iterate(monotonic_now) will return 1 and set sleep_time() to something
  // within interval of monotonic_now.
  void Reset(const monotonic_clock::time_point monotonic_now) {
    Iterate(monotonic_now - interval_);
  }

  // Calculates the next time to run after monotonic_now.
  // The result can be retrieved with sleep_time().
  // Returns the number of iterations which have passed (1 if this is called
  // often enough). This can be < 1 iff monotonic_now goes backwards between
  // calls.
  int Iterate(const monotonic_clock::time_point monotonic_now =
                  monotonic_clock::now());

  // Sleeps until the next time and returns the number of iterations which have
  // passed.
  int SleepUntilNext() {
    const int r = Iterate(monotonic_clock::now());
    ::std::this_thread::sleep_until(sleep_time());
    return r;
  }

  monotonic_clock::time_point sleep_time() const { return last_time_; }

  monotonic_clock::duration interval() const { return interval_; }
  monotonic_clock::duration offset() const { return offset_; }

 private:
  monotonic_clock::duration interval_, offset_;

  // The time we most recently slept until.
  monotonic_clock::time_point last_time_ = monotonic_clock::epoch();
};

}  // namespace time
}  // namespace aos

#endif  // AOS_UTIL_PHASED_LOOP_H_
