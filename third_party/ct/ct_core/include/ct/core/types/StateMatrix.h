/**********************************************************************************************************************
This file is part of the Control Toolbox (https://adrlab.bitbucket.io/ct), copyright by ETH Zurich, Google Inc.
Authors:  Michael Neunert, Markus Giftthaler, Markus Stäuble, Diego Pardo, Farbod Farshidian
Licensed under Apache2 license (see LICENSE file in main directory)
**********************************************************************************************************************/

#pragma once

namespace ct {
namespace core {

template <size_t STATE_DIM, class SCALAR = double>
class StateMatrix : public Eigen::Matrix<SCALAR, STATE_DIM, STATE_DIM>
{
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    StateMatrix(){};
    virtual ~StateMatrix(){};

    typedef Eigen::Matrix<SCALAR, STATE_DIM, STATE_DIM> Base;

    //! This constructor allows you to construct MyVectorType from Eigen expressions
    template <typename OtherDerived>
    StateMatrix(const Eigen::MatrixBase<OtherDerived>& other) : Base(other)
    {
    }
    //! This method allows you to assign Eigen expressions to MyVectorType
    template <typename OtherDerived>
    StateMatrix& operator=(const Eigen::MatrixBase<OtherDerived>& other)
    {
        this->Base::operator=(other);
        return *this;
    }
    //! get underlying Eigen type
    Base& toImplementation() { return *this; }
    //! get const underlying Eigen type
    const Base& toImplementation() const { return *this; }
};

} /* namespace core */
} /* namespace ct */
