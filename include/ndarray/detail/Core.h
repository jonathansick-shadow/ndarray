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
#ifndef NDARRAY_DETAIL_Core_h_INCLUDED
#define NDARRAY_DETAIL_Core_h_INCLUDED

/**
 * @file ndarray/detail/Core.h 
 *
 * @brief Definitions for Core.
 */

#include <boost/intrusive_ptr.hpp>
#include <boost/mpl/int.hpp>
#include "ndarray/Vector.h"
#include "ndarray/Manager.h"

namespace ndarray {
namespace detail {

/**
 *  @internal
 *  @brief Internal data class for Array.
 *
 *  @ingroup ndarrayInternalGroup
 *
 *  Core holds the shape, stride, and ownership data for an Array.
 *  A Core maintains its own reference count and can be shared
 *  by multiple Arrays via a const boost::intrusive pointer.
 *  
 *  Because a Core with N dimensions inherits from
 *  a Core with N-1 dimensions, subarrays can share a Core with
 *  their parants.
 *
 *  Core objects are never const; even an Array with a const
 *  template parameter holds a Core with a non-const template
 *  parameter.
 */
template <int N>
class Core : public Core<N-1> {
public:
    typedef boost::mpl::int_<N> ND;                    ///< number of dimensions
    typedef Core<N-1> Super;                           ///< base class
    typedef boost::intrusive_ptr<Core> Ptr;            ///< intrusive_ptr to Core
    typedef boost::intrusive_ptr<Core const> ConstPtr; ///< const intrusive_ptr to Core

    /// @brief Create a Core::Ptr with the given shape, strides, and manager.
    template <int M>
    static Ptr create(
        Vector<int,M> const & shape,
        Vector<int,M> const & strides, 
        Manager::Ptr const & manager = Manager::Ptr()
    ) {
        return Ptr(new Core(shape, strides, manager), false);
    }        

    /// @brief Create a Core::Ptr with the given shape and manager with contiguous strides.
    template <int M>
    static Ptr create(
        Vector<int,M> const & shape,
        DataOrderEnum order,
        Manager::Ptr const & manager = Manager::Ptr()
    ) {
        if (order == ROW_MAJOR) {
            return Ptr(new Core(shape, manager), false);
        } else {
            return Ptr(new Core(shape, 1, manager), false);
        }
    }

    /// @brief Create a Core::Ptr with the given manager and zero shape and strides.
    static Ptr create(
        Manager::Ptr const & manager = Manager::Ptr()
    ) {
        return Ptr(new Core(manager), false);
    }

    Ptr copy() const { return Ptr(new Core(*this)); }

    /// @brief Return the size of the Nth dimension.
    int getSize() const { return _size; }

    /// @brief Return the stride of the Nth dimension.
    int getStride() const { return _stride; }

    /// @brief Set the size of the Nth dimension.
    void setSize(int size) { _size = size; }

    /// @brief Set the stride of the Nth dimension.
    void setStride(int stride) { _stride = stride; }

    /// @brief Recursively compute the offset to an element.
    template <int M>
    int computeOffset(Vector<int,M> const & index) const {
        return index[M-N] * this->getStride() + Super::computeOffset(index);
    }

    /// @brief Recursively fill a shape vector.
    template <int M>
    void fillShape(Vector<int,M> & shape) const {
        shape[M-N] = this->getSize();
        Super::fillShape(shape);
    }

    /// @brief Recursively fill a strides vector.
    template <int M>
    void fillStrides(Vector<int,M> & strides) const {
        strides[M-N] = this->getStride();
        Super::fillStrides(strides);
    }

    /// @brief Recursively determine the total number of elements.
    int getNumElements() const {
        return getSize() * Super::getNumElements();
    }
    
protected:

    // Explicit strides
    template <int M>
    Core (
        Vector<int,M> const & shape,
        Vector<int,M> const & strides, 
        Manager::Ptr const & manager
    ) : Super(shape, strides, manager), _size(shape[M-N]), _stride(strides[M-N]) {}

    // Row-major strides
    template <int M>
    Core (
        Vector<int,M> const & shape,
        Manager::Ptr const & manager
    ) : Super(shape, manager), _size(shape[M-N]), _stride(Super::getStride() * Super::getSize()) {}

    // Column-major strides
    template <int M>
    Core (
        Vector<int,M> const & shape,
        int stride,
        Manager::Ptr const & manager
    ) : Super(shape, stride * shape[M-N], manager), _size(shape[M-N]), _stride(stride) {}

    // Zero shape and strides
    Core (
        Manager::Ptr const & manager
    ) : Super(manager), _size(0), _stride(0) {}

    Core(Core const & other) : Super(other), _size(other._size), _stride(other._stride) {}

private:
    int _size;
    int _stride;
};

/**
 *  @internal
 *  @brief Internal data class for Array, 0-D specialization.
 *
 *  @ingroup ndarrayInternalGroup
 *
 *  The 0-D Core has size and stride == 1 and holds the reference
 *  count and manager; it is the base class for all other Cores.
 */
template <>
class Core<0> {
public:
    typedef boost::mpl::int_<0> ND;
    typedef boost::intrusive_ptr<Core> Ptr;
    typedef boost::intrusive_ptr<Core const> ConstPtr;

    friend inline void intrusive_ptr_add_ref(Core const * core) {
        ++core->_rc;
    }
 
    friend inline void intrusive_ptr_release(Core const * core) {
        if ((--core->_rc)==0) delete core;
    }

    Ptr copy() const { return Ptr(new Core(*this)); }

    int getSize() const { return 1; }
    int getStride() const { return 1; }

    /// @brief Recursively compute the offset to an element.
    template <int M>
    int computeOffset(Vector<int,M> const & index) const { return 0; }

    /// @brief Return the Manager that determines the lifetime of the array data.
    Manager::Ptr getManager() const { return _manager; }

    /// @brief Set the Manager that determines the lifetime of the array data.
    void setManager(Manager::Ptr const & manager) { _manager = manager; }

    /// @brief Recursively fill a shape vector.
    template <int M>
    void fillShape(Vector<int,M> const & shape) const {}

    /// @brief Recursively fill a strides vector.
    template <int M>
    void fillStrides(Vector<int,M> const & strides) const {}

    /// @brief Recursively determine the total number of elements.
    int getNumElements() const { return 1; }

    /// @brief Return the reference count (for debugging purposes).
    int getRC() const { return _rc; }

    /// @brief Return true if the Core and Manager reference counts are 1 and the manager is unique.
    bool isUnique() const { return (_rc == 1) && (_manager->getRC() == 1) && _manager->isUnique(); }

protected:

    virtual ~Core() {}

    template <int M>
    Core(
        Vector<int,M> const & shape,
        Vector<int,M> const & strides, 
        Manager::Ptr const & manager
    ) : _manager(manager), _rc(1) {}

    template <int M>
    Core(
        Vector<int,M> const & shape,
        Manager::Ptr const & manager
    ) : _manager(manager), _rc(1) {}

    template <int M>
    Core(
        Vector<int,M> const & shape,
        int stride,
        Manager::Ptr const & manager
    ) : _manager(manager), _rc(1) {}

    Core(
        Manager::Ptr const & manager
    ) : _manager(manager), _rc(1) {}

    Core(Core const & other) : _manager(other._manager), _rc(1) {}

private:
    Manager::Ptr _manager;
    mutable int _rc;
};


/**
 *  @internal @brief Cast a Core reference to a particular dimension.
 *
 *  @ingroup ndarrayInternalGroup
 */
template <int P, int N>
inline Core<N-P> const & 
getDimension(Core<N> const & core) { return core; }

/**
 *  @internal @brief Cast a Core smart pointer to a particular dimension.
 *
 *  @ingroup ndarrayInternalGroup
 */
template <int P, int N>
inline typename Core<N-P>::Ptr 
getDimension(typename Core<N>::Ptr const & core) { return core; }

} // namespace detail
} // namespace ndarray

#endif // !NDARRAY_DETAIL_Core_h_INCLUDED
