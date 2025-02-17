/**********************************************************************************************************************
This file is part of the Control Toolbox (https://adrlab.bitbucket.io/ct), copyright by ETH Zurich, Google Inc.
Authors:  Michael Neunert, Markus Giftthaler, Markus Stäuble, Diego Pardo, Farbod Farshidian
Licensed under Apache2 license (see LICENSE file in main directory)
**********************************************************************************************************************/

/*! \file
 *	\brief		Auto-generated code for computing the derivative of qdd with respect to state and input
 *  \author	    Michael Neunert
 *
 *  @example 	timingFullvsSeparateJacobian.cpp
 *  This is an example of how to use the generated code.
 *
 *  @example 	timingFullJacobian.cpp
 *  This is an example of how to use the generated code.
 */

#pragma once

#include <array>
#include <Eigen/Core>

namespace ct_HyA {

Eigen::Matrix<double, 12 + 6, 6> computeFullJacobianCodegen(const Eigen::Matrix<double, 12, 1>& state,
    const Eigen::Matrix<double, 6, 1>& tau);
}
