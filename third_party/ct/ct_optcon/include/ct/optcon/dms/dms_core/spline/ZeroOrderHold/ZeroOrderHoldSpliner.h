/**********************************************************************************************************************
This file is part of the Control Toolbox (https://adrlab.bitbucket.io/ct), copyright by ETH Zurich, Google Inc.
Authors:  Michael Neunert, Markus Giftthaler, Markus Stäuble, Diego Pardo, Farbod Farshidian
Licensed under Apache2 license (see LICENSE file in main directory)
**********************************************************************************************************************/

#pragma once

#include "ct/optcon/dms/dms_core/spline/SplinerBase.h"
#include <ct/optcon/dms/dms_core/TimeGrid.h>

namespace ct {
namespace optcon {

/**
 * @ingroup    DMS
 *
 * @brief      The spline implementation for the zero order hold spliner
 *
 * @tparam     T     The vector type to be splined
 */
template <class T, typename SCALAR = double>
class ZeroOrderHoldSpliner : public SplinerBase<T, SCALAR>
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    typedef T vector_t;
    typedef Eigen::Matrix<SCALAR, T::DIM, T::DIM> matrix_t;
    typedef std::vector<vector_t, Eigen::aligned_allocator<vector_t>> vector_array_t;

    ZeroOrderHoldSpliner() = delete;

    /**
	 * @brief      Custom constructor
	 *
	 * @param[in]  grid  The DMS timegrid
	 */
    ZeroOrderHoldSpliner(std::shared_ptr<tpl::TimeGrid<SCALAR>> grid) : timeGrid_(grid) {}
    virtual ~ZeroOrderHoldSpliner() {}
    void computeSpline(const vector_array_t& points) override { zOholds_ = points; }
    // evaluate spline and return vector at interpolation time
    virtual vector_t evalSpline(const SCALAR time, const size_t shotIdx) override
    {
        assert(shotIdx < zOholds_.size());
        assert(zOholds_[shotIdx] == zOholds_[shotIdx]);
        return zOholds_[shotIdx];
    }

    virtual vector_t splineDerivative_t(const SCALAR time, const size_t shotIdx) const override
    {
        return vector_t::Zero();
    }

    virtual vector_t splineDerivative_h_i(const SCALAR time, const size_t shotIdx) const override
    {
        return vector_t::Zero();
    }

    virtual matrix_t splineDerivative_q_i(const SCALAR time, const size_t shotIdx) const override
    {
        return matrix_t::Identity();
    }

    virtual matrix_t splineDerivative_q_iplus1(const SCALAR time, const size_t shotIdx) const override
    {
        return matrix_t::Zero();
    }


private:
    // zero order hold variables
    vector_array_t zOholds_;

    std::shared_ptr<tpl::TimeGrid<SCALAR>> timeGrid_;
};

}  // namespace optcon
}  // namespace ct
