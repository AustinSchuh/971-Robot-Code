/**********************************************************************************************************************
This file is part of the Control Toolbox (https://adrlab.bitbucket.io/ct), copyright by ETH Zurich, Google Inc.
Authors:  Michael Neunert, Markus Giftthaler, Markus Stäuble, Diego Pardo, Farbod Farshidian
Licensed under Apache2 license (see LICENSE file in main directory)
**********************************************************************************************************************/
// Bring in gtest
#include <gtest/gtest.h>

#include <cppad/cg.hpp>
#include <cppad/cppad.hpp>
#include <cppad/example/cppad_eigen.hpp>
#include <cppad/example/eigen_mat_inv.hpp>


#include <ct/core/core.h>

// Bring in gtest
#include <gtest/gtest.h>

using namespace ct::core;
using std::shared_ptr;


// codegen tests cannot run in parallel. Thus we include all tests here and run them together

#include "math/JacobianCGTest.h"
#include "system/ADCodegenLinearizerTest.h"
#include "system/DiscreteSystemLinearizerADCGTest.h"


/*!
 *  \example CodegenTests.cpp
 *
 *  This unit test serves as example how to use automatic code generation with CppAD
 */
int main(int argc, char **argv)
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
