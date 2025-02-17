#include "frc971/control_loops/quaternion_utils.h"

#include "Eigen/Dense"
#include "Eigen/Geometry"

namespace frc971 {
namespace controls {

Eigen::Matrix<double, 4, 1> ToQuaternionFromRotationVector(
    const Eigen::Matrix<double, 3, 1> &X, const double max_angle_cap) {
  const double unclipped_angle = X.norm();
  const double angle_scalar =
      (unclipped_angle > max_angle_cap) ? max_angle_cap / unclipped_angle : 1.0;
  const double angle = unclipped_angle * angle_scalar;
  const double half_angle = angle * 0.5;

  const double half_angle_squared = half_angle * half_angle;

  // sin(x)/x = 1
  double sinx_x = 1.0;

  // - x^2/3!
  double value = half_angle_squared / 6.0;
  sinx_x -= value;

  // + x^4/5!
  value = value * half_angle_squared / 20.0;
  sinx_x += value;

  // - x^6/7!
  value = value * half_angle_squared / (6.0 * 7.0);
  sinx_x -= value;

  // + x^8/9!
  value = value * half_angle_squared / (8.0 * 9.0);
  sinx_x += value;

  // - x^10/11!
  value = value * half_angle_squared / (10.0 * 11.0);
  sinx_x -= value;

  // + x^12/13!
  value = value * half_angle_squared / (12.0 * 13.0);
  sinx_x += value;

  // - x^14/15!
  value = value * half_angle_squared / (14.0 * 15.0);
  sinx_x -= value;

  // + x^16/17!
  value = value * half_angle_squared / (16.0 * 17.0);
  sinx_x += value;

  // To plot the residual in matplotlib, run:
  // import numpy
  // import scipy
  // from matplotlib import pyplot
  // x = numpy.arange(-numpy.pi, numpy.pi, 0.01)
  // pyplot.plot(x, 1 - x**2 / scipy.misc.factorial(3) +
  //                   x**4 / scipy.misc.factorial(5) -
  //                   x**6 / scipy.misc.factorial(7) +
  //                   x**8 / scipy.misc.factorial(9) -
  //                   x ** 10 / scipy.misc.factorial(11) +
  //                   x ** 12 / scipy.misc.factorial(13) -
  //                   x ** 14 / scipy.misc.factorial(15) +
  //                   x ** 16 / scipy.misc.factorial(17) -
  //                   numpy.sin(x) / x)

  const double scalar = sinx_x * 0.5;

  Eigen::Matrix<double, 4, 1> result;
  result.block<3, 1>(0, 0) = X * scalar * angle_scalar;
  result(3, 0) = std::cos(half_angle);
  return result;
}

inline Eigen::Matrix<double, 4, 1> MaybeFlipX(
    const Eigen::Matrix<double, 4, 1> &X) {
  if (X(3, 0) < 0.0) {
    return -X;
  } else {
    return X;
  }
}

Eigen::Matrix<double, 3, 1> ToRotationVectorFromQuaternion(
    const Eigen::Matrix<double, 4, 1> &X) {
  // TODO(austin): Verify we still need it.
  const Eigen::Matrix<double, 4, 1> corrected_X = MaybeFlipX(X);
  const double half_angle =
      std::atan2(corrected_X.block<3, 1>(0, 0).norm(), corrected_X(3, 0));

  const double half_angle_squared = half_angle * half_angle;

  // TODO(austin): We are doing a division at the end of this. Do the taylor
  // series expansion of x/sin(x) instead to avoid this.

  // sin(x)/x = 1
  double sinx_x = 1.0;

  // - x^2/3!
  double value = half_angle_squared / 6.0;
  sinx_x -= value;

  // + x^4/5!
  value = value * half_angle_squared / 20.0;
  sinx_x += value;

  // - x^6/7!
  value = value * half_angle_squared / (6.0 * 7.0);
  sinx_x -= value;

  // + x^8/9!
  value = value * half_angle_squared / (8.0 * 9.0);
  sinx_x += value;

  // - x^10/11!
  value = value * half_angle_squared / (10.0 * 11.0);
  sinx_x -= value;

  // + x^12/13!
  value = value * half_angle_squared / (12.0 * 13.0);
  sinx_x += value;

  // - x^14/15!
  value = value * half_angle_squared / (14.0 * 15.0);
  sinx_x -= value;

  // + x^16/17!
  value = value * half_angle_squared / (16.0 * 17.0);
  sinx_x += value;

  const double scalar = 2.0 / sinx_x;

  return corrected_X.block<3, 1>(0, 0) * scalar;
}

}  // namespace controls
}  // namespace frc971
