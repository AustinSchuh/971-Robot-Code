/**********************************************************************************************************************
This file is part of the Control Toolbox (https://adrlab.bitbucket.io/ct), copyright by ETH Zurich, Google Inc.
Authors:  Michael Neunert, Markus Giftthaler, Markus Stäuble, Diego Pardo, Farbod Farshidian
Licensed under Apache2 license (see LICENSE file in main directory)
 **********************************************************************************************************************/

#pragma once

namespace ct {
namespace optcon {

template <size_t STATE_DIM, size_t CONTROL_DIM, typename SCALAR>
DARE<STATE_DIM, CONTROL_DIM, SCALAR>::DARE()
{
}

template <size_t STATE_DIM, size_t CONTROL_DIM, typename SCALAR>
typename DARE<STATE_DIM, CONTROL_DIM, SCALAR>::state_matrix_t
DARE<STATE_DIM, CONTROL_DIM, SCALAR>::computeSteadyStateRiccatiMatrix(const state_matrix_t& Q,
    const control_matrix_t& R,
    const state_matrix_t& A,
    const control_gain_matrix_t& B,
    control_feedback_t& K,
    bool verbose,
    const SCALAR eps,
    size_t maxIter)
{
    // If initialized to zero, in the next iteration P becomes Q. Directly initializing to Q avoids the extra iteration.
    return computeSteadyStateRiccatiMatrix(Q, R, A, B, Q, K, verbose, eps, maxIter);
}

template <size_t STATE_DIM, size_t CONTROL_DIM, typename SCALAR>
typename DARE<STATE_DIM, CONTROL_DIM, SCALAR>::state_matrix_t
DARE<STATE_DIM, CONTROL_DIM, SCALAR>::computeSteadyStateRiccatiMatrix(const state_matrix_t& Q,
    const control_matrix_t& R,
    const state_matrix_t& A,
    const control_gain_matrix_t& B,
    state_matrix_t P,
    control_feedback_t& K,
    bool verbose,
    const SCALAR eps,
    size_t maxIter)
{
    state_matrix_t P_prev;
    size_t numIter = 0;

    SCALAR diff = 1;

    while (diff >= eps && numIter < maxIter)
    {
        P_prev = P;
        dynamicRDE_.iterateRobust(Q, R, A, B, P, K);
        diff = (P - P_prev).cwiseAbs().maxCoeff();
        if (!K.allFinite())
            throw std::runtime_error("DARE : Failed to converge");
        numIter++;
    }

    if (diff >= eps) throw std::runtime_error("DARE : Failed to converge");

    if (verbose)
    {
        std::cout << "DARE : converged after " << numIter << " iterations out of a maximum of " << maxIter << std::endl;
        std::cout << "Resulting K: " << K << std::endl;
    }

    return P;
}

}  // namespace optcon
}  // namespace ct
