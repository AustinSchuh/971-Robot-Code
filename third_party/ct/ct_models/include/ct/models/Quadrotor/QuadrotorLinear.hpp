/**********************************************************************************************************************
This file is part of the Control Toolbox (https://adrlab.bitbucket.io/ct), copyright by ETH Zurich, Google Inc.
Authors:  Michael Neunert, Markus Giftthaler, Markus Stäuble, Diego Pardo, Farbod Farshidian
Licensed under Apache2 license (see LICENSE file in main directory)
**********************************************************************************************************************/

#pragma once


#include <Eigen/Dense>
#include <ct/core/core.h>
#include "Quadrotor.hpp"


namespace ct {
namespace models {

class QuadrotorLinear : public ct::core::LinearSystem<quadrotor::nStates, quadrotor::nControls>
{
public:
    typedef ct::core::StateVector<quadrotor::nStates> state_vector_t;
    typedef ct::core::ControlVector<quadrotor::nControls> control_vector_t;

    typedef Eigen::Matrix<double, quadrotor::nStates, quadrotor::nStates> state_matrix_t;
    typedef Eigen::Matrix<double, quadrotor::nStates, quadrotor::nControls> state_control_matrix_t;


    virtual QuadrotorLinear* clone() const override { return new QuadrotorLinear(*this); }
    virtual const state_matrix_t& getDerivativeState(const state_vector_t& x,
        const control_vector_t& u,
        const ct::core::Time t = 0.0) override
    {
        A_ = A_quadrotor(x, u);
        return A_;
    }

    virtual const state_control_matrix_t& getDerivativeControl(const state_vector_t& x,
        const control_vector_t& u,
        const ct::core::Time t = 0.0) override
    {
        B_ = B_quadrotor(x, u);
        return B_;
    }

private:
    state_matrix_t A_;
    state_control_matrix_t B_;
};
}
}
