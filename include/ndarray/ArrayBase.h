// -*- c++ -*-
/* 
 * LSST Data Management System
 * Copyright 2008, 2009, 2010, 2011 LSST Corporation.
 * 
 * This product includes software developed by the
 * LSST Project (http://www.lsst.org/).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the LSST License Statement and 
 * the GNU General Public License along with this program.  If not, 
 * see <http://www.lsstcorp.org/LegalNotices/>.
 */
#ifndef NDARRAY_ArrayBase_h_INCLUDED
#define NDARRAY_ArrayBase_h_INCLUDED

/** 
 *  @file ndarray/ArrayBase.h
 *
 *  @brief Definitions for ArrayBase.
 */


#include <boost/iterator/counting_iterator.hpp>

#include "ndarray/ExpressionBase.h"
#include "ndarray/Vector.h"
#include "ndarray/detail/Core.h"
#include "ndarray/detail/NestedIterator.h"
#include "ndarray/detail/StridedIterator.h"
#include "ndarray/detail/ArrayAccess.h"
#include "ndarray/detail/ViewBuilder.h"
#include "ndarray/ArrayTraits.h"
#include "ndarray/eigen_fwd.h"

namespace ndarray {

/**
 *  @class ArrayBase
 *  @brief CRTP implementation for Array and ArrayRef.
 *
 *  @ingroup MainGroup
 *
 *  Implements member functions that need specialization for 1D arrays.
 */
template <typename Derived>
class ArrayBase : public ExpressionBase<Derived> {
protected:
    typedef ExpressionTraits<Derived> Traits;
    typedef typename Traits::Core Core;
    typedef typename Traits::CorePtr CorePtr;
public:
    /// @brief Data type of array elements.
    typedef typename Traits::Element Element;
    /// @brief Nested array or element iterator.
    typedef typename Traits::Iterator Iterator;
    /// @brief Nested array or element reference.
    typedef typename Traits::Reference Reference;
    /// @brief Nested array or element value type.
    typedef typename Traits::Value Value;
    /// @brief Number of dimensions (boost::mpl::int_).
    typedef typename Traits::ND ND;
    /// @brief Number of guaranteed row-major contiguous dimensions, counted from the end (boost::mpl::int_).
    typedef typename Traits::RMC RMC;
    /// @brief Vector type for N-dimensional indices.
    typedef Vector<int,ND::value> Index;
    /// @brief ArrayRef to a reverse-ordered contiguous array; the result of a call to transpose().
    typedef ArrayRef<Element,ND::value,-RMC::value> FullTranspose;
    /// @brief ArrayRef to a noncontiguous array; the result of a call to transpose(...).
    typedef ArrayRef<Element,ND::value,0> Transpose;
    /// @brief The corresponding Array type.
    typedef Array<Element,ND::value,RMC::value> Shallow;
    /// @brief The corresponding ArrayRef type.
    typedef ArrayRef<Element,ND::value,RMC::value> Deep;

    /// @brief Return a single subarray.
    Reference operator[](int n) const {
        return Traits::makeReference(
            this->_data + n * this->template getStride<0>(),
            this->_core
        );
    }

    /// @brief Return a single element from the array.
    Element & operator[](Index const & i) const {
        return *(this->_data + this->_core->template computeOffset(i));
    }

    /// @brief Return an Iterator to the beginning of the array.
    Iterator begin() const {
        return Traits::makeIterator(
            this->_data,
            this->_core,
            this->template getStride<0>()
        );
    }

    /// @brief Return an Iterator to one past the end of the array.
    Iterator end() const {
        return Traits::makeIterator(
            this->_data + this->template getSize<0>() * this->template getStride<0>(), 
            this->_core,
            this->template getStride<0>()
        );
    }

    /// @brief Return a raw pointer to the first element of the array.
    Element * getData() const { return _data; }
    
    /// @brief Return true if the array has a null data point.
    bool isEmpty() const { return _data == 0; }

    /// @brief Return the opaque object responsible for memory management.
    Manager::Ptr getManager() const { return this->_core->getManager(); }

    /// @brief Return the size of a specific dimension.
    template <int P> int getSize() const {
        return detail::getDimension<P>(*this->_core).getSize();
    }

    /// @brief Return the stride in a specific dimension.
    template <int P> int getStride() const {
        return detail::getDimension<P>(*this->_core).getStride();
    }

    /// @brief Return a Vector of the sizes of all dimensions.
    Index getShape() const { Index r; this->_core->fillShape(r); return r; }

    /// @brief Return a Vector of the strides of all dimensions.
    Index getStrides() const { Index r; this->_core->fillStrides(r); return r; }

    /// @brief Return the total number of elements in the array.
    int getNumElements() const { return this->_core->getNumElements(); }

    /// @brief Return a view of the array with the order of the dimensions reversed.
    FullTranspose transpose() const {
        Index shape = getShape();
        Index strides = getStrides();
        for (int n=0; n < ND::value / 2; ++n) {
            std::swap(shape[n], shape[ND::value-n-1]);
            std::swap(strides[n], strides[ND::value-n-1]);
        }
        return FullTranspose(
            getData(),
            Core::create(shape, strides, getManager())
        );
    }

    /// @brief Return a view of the array with the dimensions permuted.
    Transpose transpose(Index const & order) const {
        Index newShape;
        Index newStrides;
        Index oldShape = getShape();
        Index oldStrides = getStrides();
        for (int n=0; n < ND::value; ++n) {
            newShape[n] = oldShape[order[n]];
            newStrides[n] = oldStrides[order[n]];
        }
        return Transpose(
            getData(), 
            Core::create(newShape, newStrides, getManager())
        );
    }

    /// @brief Return a Array view to this.
    Shallow const shallow() const { return Shallow(this->getSelf()); }

    /// @brief Return an ArrayRef view to this.
    Deep const deep() const { return Deep(this->getSelf()); }
    
    //@{
    /**
     *  @name Eigen3 Interface
     *
     *  These methods return Eigen3 views to the array.  Template
     *  parameters optionally control the expression type (Matrix/Array) and
     *  the compile-time dimensions.
     *
     *  The inline implementation is included by ndarray/eigen.h.
     */
    template <typename XprKind, int Rows, int Cols>
    EigenView<Element,ND::value,RMC::value,XprKind,Rows,Cols> asEigen() const;

    template <typename XprKind>
    EigenView<Element,ND::value,RMC::value,XprKind> asEigen() const;

    template <int Rows, int Cols>
    EigenView<Element,ND::value,RMC::value,Eigen::MatrixXpr,Rows,Cols> asEigen() const;

    EigenView<Element,ND::value,RMC::value,Eigen::MatrixXpr> asEigen() const;
    //@}

    /// @brief A template metafunction class to determine the result of a view indexing operation.
    template <typename View_>
    struct ResultOf {
        typedef Element Element_;
        typedef typename detail::ViewTraits<ND::value, RMC::value, typename View_::Sequence>::ND ND_;
        typedef typename detail::ViewTraits<ND::value, RMC::value, typename View_::Sequence>::RMC RMC_;
        typedef ArrayRef<Element_,ND_::value,RMC_::value> Type;
        typedef Array<Element_,ND_::value,RMC_::value> Value;
    };

    /// @brief Return a general view into this array (see @ref ndarrayTutorial).
    template <typename Seq>
    typename ResultOf< View<Seq> >::Type
    operator[](View<Seq> const & def) const {
        return detail::buildView(this->getSelf(), def._seq);
    }

protected:
    template <typename T_, int N_, int C_> friend class Array;
    template <typename T_, int N_, int C_> friend class ArrayRef;
    template <typename T_, int N_, int C_> friend struct ArrayTraits;
    template <typename T_, int N_, int C_> friend class detail::NestedIterator;
    template <typename Derived_> friend class ArrayBase;
    template <typename Array_> friend class detail::ArrayAccess;

    Element * _data;
    CorePtr _core;

    void operator=(ArrayBase const & other) {
        _data = other._data;
        _core = other._core;
    }

    ArrayBase(Element * data, CorePtr const & core) : _data(data), _core(core) {}
};

} // namespace ndarray

#endif // !NDARRAY_ArrayBase_h_INCLUDED
