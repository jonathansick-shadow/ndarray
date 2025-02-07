// -*- lsst-c++ -*-
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
changecom(`###')dnl
define(`VECTOR_ASSIGN',
`
    /// @brief Augmented $1 assignment from another vector.
    template <typename U>
    typename boost::enable_if<boost::is_convertible<U,T>,Vector&>::type
    operator $1 (Vector<U,N> const & other) {
        typename Vector<U,N>::ConstIterator j = other.begin();
        for (Iterator i = begin(); i != end(); ++i, ++j) (*i) $1 (*j);
        return *this;
    }
    /// @brief Augmented $1 assignment from a scalar.
    template <typename U>
    typename boost::enable_if<boost::is_convertible<U,T>,Vector&>::type
    operator $1 (U scalar) {
        for (Iterator i = begin(); i != end(); ++i) (*i) $1 scalar;
        return *this;
    }')dnl
define(`VECTOR_BINARY_OP',
`
    /// @brief Operator overload for Vector $1 Vector.
    template <typename T, typename U, int N>
    Vector<typename Promote<T,U>::Type,N>
    operator $1(Vector<T,N> const & a, Vector<U,N> const & b) {
        Vector<typename Promote<T,U>::Type,N> r(a);
        return r $1= b;
    }
    /** @brief Operator overload for Vector $1 Scalar. */
    template <typename T, typename U, int N>
    Vector<typename Promote<T,U>::Type,N>
    operator $1(Vector<T,N> const & a, U b) {
        Vector<typename Promote<T,U>::Type,N> r(a);
        return r $1= b;
    }
    /** @brief Operator overload for Scalar $1 Vector. */
    template <typename T, typename U, int N>
    Vector<typename Promote<T,U>::Type,N>
    operator $1(U a, Vector<T,N> const & b) {
        Vector<typename Promote<T,U>::Type,N> r(a);
        return r $1= b;
    }')dnl
define(`VECTOR_TYPEDEFS',
`
    typedef T Element;
    typedef T Value;
    typedef T & Reference;
    typedef T const & ConstReference;
    typedef T * Iterator;
    typedef T const * ConstIterator;
    
    typedef Value value_type;
    typedef Iterator iterator;
    typedef ConstIterator const_iterator;
    typedef Reference reference;
    typedef ConstReference const_reference;
    typedef boost::reverse_iterator<T*> reverse_iterator;
    typedef boost::reverse_iterator<const T*> const_reverse_iterator;
    typedef T * pointer;
    typedef int difference_type;
    typedef int size_type;
')dnl
#ifndef NDARRAY_Vector_h_INCLUDED
#define NDARRAY_Vector_h_INCLUDED

/// @file ndarray/Vector.h Definition for Vector.

#include <boost/type_traits/is_arithmetic.hpp>
#include <boost/iterator/reverse_iterator.hpp>
#include <boost/mpl/int.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#include <boost/preprocessor/repetition/enum.hpp>

#include <iostream>

#include "ndarray_fwd.h"
#include "ndarray/types.h"

/// \cond MACROS
#define NDARRAY_MAKE_VECTOR_MAX 8

#define NDARRAY_MAKE_VECTOR_ARG_SPEC(Z,I,DATA) T v ## I
#define NDARRAY_MAKE_VECTOR_SET_SPEC(Z,I,DATA) r[I] = v ## I;

#define NDARRAY_MAKE_VECTOR_SPEC(Z,N,DATA)                      \
    template <typename T>                                       \
    inline Vector<T,N> makeVector(                              \
        BOOST_PP_ENUM(N,NDARRAY_MAKE_VECTOR_ARG_SPEC,unused)    \
    ) {                                                         \
        Vector<T,N> r;                                          \
        BOOST_PP_REPEAT(N,NDARRAY_MAKE_VECTOR_SET_SPEC,unused)  \
        return r;                                               \
    }

/// \endcond

namespace ndarray {

namespace detail {

template <typename T, bool isArithmetic=boost::is_arithmetic<T>::value>
struct DefaultValue {
    static T get() { return T(); }
};

template <typename T>
struct DefaultValue<T,true> {
    static T get() { return T(0); }
};

} // namespace detail

/// \addtogroup ndarrayVectorGroup
/// @{

/** 
 *  @class Vector
 *  @brief A fixed-size 1D array class.
 *
 *  Vector (with T==int) is primarily used as the data
 *  type for the shape and strides attributes of Array.
 *  
 *  Vector is implemented almost exactly as a non-aggregate
 *  boost::array, but with the addition of mathematical
 *  operators and a few other utility functions.
 */
template <
    typename T, ///< Data type.
    int N       ///< Number of elements.
    >
struct Vector {
    VECTOR_TYPEDEFS

    typedef boost::mpl::int_<N> ND;

    size_type size() const { return N; }           ///< @brief Return the size of the Vector.
    size_type max_size() const { return N; }       ///< @brief Return the size of the Vector.
    bool empty() const { return N==0; }            ///< @brief Return true if size() == 0.
    /// @brief Return an iterator to the beginning of the Vector.
    iterator begin() { return elems; }
    /// @brief Return a const_iterator to the beginning of the Vector.
    const_iterator begin() const { return elems; }
    /// @brief Return an iterator to the end of the Vector.
    iterator end() { return elems+N; }
    /// @brief Return a const_iterator to the end of the Vector.
    const_iterator end() const { return elems+N; }
    /// @brief Return a reverse_iterator to the beginning of the reversed Vector.
    reverse_iterator rbegin() { return reverse_iterator(end()); }
    /// @brief Return a const_reverse_iterator to the beginning of the reversed Vector.
    const_reverse_iterator rbegin() const { return const_reverse_iterator(end()); }
    /// @brief Return a reverse_iterator to the end of the reversed Vector.
    reverse_iterator rend() { return reverse_iterator(begin()); }
    /// @brief Return a const_reverse_iterator to the end of the reversed Vector.
    const_reverse_iterator rend() const { return const_reverse_iterator(begin()); }

    /// @brief Return a reference to the first element.
    reference front() { return *elems; }
    /// @brief Return a reference to the last element.
    reference back() { return *(elems+N-1); }
    /// @brief Return a const_reference to the first element.
    const_reference front() const { return *elems; }
    /// @brief Return a const_reference to the last element.
    const_reference back() const { return *(elems+N-1); }

    /// @brief Return a reference to the element with the given index.
    reference operator[](int i) { return elems[i]; }
    /// @brief Return a const_reference to the element with the given index.
    const_reference operator[](int i) const { return elems[i]; }

    /// @brief Create a new Vector that is a subset of this.
    template <int Start, int Stop>
    Vector<T,Stop-Start> getRange() const {
        Vector<T,Stop-Start> r;
        std::copy(begin() + Start, begin()+Stop, r.begin());
        return r;
    }

    /// @brief Create a new Vector from the first M elements of this.
    template <int M> Vector<T,M> first() const {
        Vector<T,M> r;
        std::copy(begin(), begin() + M, r.begin());
        return r;
    }

    /// @brief Create a new Vector from the last M elements of this.
    template <int M> Vector<T,M> last() const {
        Vector<T,M> r;
        std::copy(begin() + (N - M), begin() + N, r.begin());
        return r;
    }

    /** @brief Stream output. */
    friend std::ostream& operator<<(std::ostream& os, Vector<T,N> const & obj) {
        os << "(";
        std::copy(obj.begin(), obj.end(), std::ostream_iterator<T>(os,","));
        return os << ")";
    }

    /**
     *  @brief Default constructor.
     *
     *  Initializes the elements to zero.
     */
    Vector() { this->template operator=(detail::DefaultValue<T>::get()); }

    /// @brief Construct with copies of a scalar.
    template <typename U>
    explicit Vector(U scalar) {
        this->template operator=(scalar);
    }

    /// @brief Converting copy constructor.
    template <typename U>
    explicit Vector(Vector<U,N> const & other) {
        this->template operator=(other);
    }

    /// @brief Return true if elements of other are equal to the elements of this.
    bool operator==(Vector const & other) const {
        return std::equal(begin(), end(), other.begin());
    }

    /// @brief Return false if any elements of other are not equal to the elements of this.
    bool operator!=(Vector const & other) const {
        return !(*this == other);
    }

    /// @brief Return the sum of all elements.
    T sum() const {
        T r = 0;
        for (ConstIterator i = begin(); i != end(); ++i) r += (*i);
        return r;
    }

    /// @brief Return the product of all elements.
    T product() const {
        T r = 1;
        for (ConstIterator i = begin(); i != end(); ++i) r *= (*i);
        return r;
    }

    /// @brief Return a Vector with the elements reversed.
    Vector reverse() const {
        Vector r;
        std::copy(begin(), end(), r.rbegin());
        return r;
    }

    VECTOR_ASSIGN(=)
    VECTOR_ASSIGN(+=)
    VECTOR_ASSIGN(-=)
    VECTOR_ASSIGN(*=)
    VECTOR_ASSIGN(/=)
    VECTOR_ASSIGN(%=)
    VECTOR_ASSIGN(&=)
    VECTOR_ASSIGN(^=)
    VECTOR_ASSIGN(|=)
    VECTOR_ASSIGN(<<=)
    VECTOR_ASSIGN(>>=)

    T elems[N];
};

/// @brief PArtial specialization for zero-size vectors to avoid compiler errors on some platforms.
template <typename T>
struct Vector<T,0> {
    VECTOR_TYPEDEFS

    typedef boost::mpl::int_<0> ND;

    size_type size() const { return 0; }           ///< @brief Return the size of the Vector.
    size_type max_size() const { return 0; }       ///< @brief Return the size of the Vector.
    bool empty() const { return true; }            ///< @brief Return true if size() == 0.
    /// @brief Return an iterator to the beginning of the Vector.
    iterator begin() { return 0; }
    /// @brief Return a const_iterator to the beginning of the Vector.
    const_iterator begin() const { return 0; }
    /// @brief Return an iterator to the end of the Vector.
    iterator end() { return 0; }
    /// @brief Return a const_iterator to the end of the Vector.
    const_iterator end() const { return 0; }
    /// @brief Return a reverse_iterator to the beginning of the reversed Vector.
    reverse_iterator rbegin() { return reverse_iterator(); }
    /// @brief Return a const_reverse_iterator to the beginning of the reversed Vector.
    const_reverse_iterator rbegin() const { return const_reverse_iterator(); }
    /// @brief Return a reverse_iterator to the end of the reversed Vector.
    reverse_iterator rend() { return reverse_iterator(); }
    /// @brief Return a const_reverse_iterator to the end of the reversed Vector.
    const_reverse_iterator rend() const { return const_reverse_iterator(); }

    /// @brief Return a reference to the first element.
    reference front() { NDARRAY_ASSERT(false); return 0; }
    /// @brief Return a reference to the last element.
    reference back() { return NDARRAY_ASSERT(false); return 0; }
    /// @brief Return a const_reference to the first element.
    const_reference front() const { NDARRAY_ASSERT(false); return 0; }
    /// @brief Return a const_reference to the last element.
    const_reference back() const { NDARRAY_ASSERT(false); return 0; }

    /// @brief Return a reference to the element with the given index.
    reference operator[](int i) { NDARRAY_ASSERT(false); return 0; }
    /// @brief Return a const_reference to the element with the given index.
    const_reference operator[](int i) const { NDARRAY_ASSERT(false); return 0; }

    /// @brief Create a new Vector that is a subset of this.
    template <int Start, int Stop>
    Vector<T,Stop-Start> getRange() const {
        return Vector<T,Stop-Start>();
    }

    /// @brief Create a new Vector from the first M elements of this.
    template <int M> Vector<T,M> first() const {
        return Vector<T,M>();
    }

    /// @brief Create a new Vector from the last M elements of this.
    template <int M> Vector<T,M> last() const {
        return Vector<T,M>();
    }

    /** @brief Stream output. */
    friend std::ostream& operator<<(std::ostream& os, Vector<T,0> const & obj) {
        return os << "()";
    }

    /**
     *  @brief Default constructor.
     *
     *  Initializes the elements to zero.
     */
    Vector() {}

    /// @brief Construct with copies of a scalar.
    template <typename U>
    explicit Vector(U scalar) {}

    /// @brief Converting copy constructor.
    template <typename U>
    explicit Vector(Vector<U,0> const & other) {}

    /// @brief Return true if elements of other are equal to the elements of this.
    bool operator==(Vector const & other) const { return true; }

    /// @brief Return false if any elements of other are not equal to the elements of this.
    bool operator!=(Vector const & other) const { return false; }

    /// @brief Return the sum of all elements.
    T sum() const { return 0; }

    /// @brief Return the product of all elements.
    T product() const { return 1; }

    /// @brief Return a Vector with the elements reversed.
    Vector reverse() const { return Vector(); }

};


/// @brief Concatenate two Vectors into a single long Vector.
template <typename T, int N, int M>
inline Vector<T,N+M> concatenate(Vector<T,N> const & a, Vector<T,M> const & b) {
    Vector<T,N+M> r;
    std::copy(a.begin(),a.end(),r.begin());
    std::copy(b.begin(),b.end(),r.begin()+N);
    return r;
}

/// @brief Return a new Vector with the given scalar appended to the original.
template <typename T, int N>
inline Vector<T,N+1> concatenate(Vector<T,N> const & a, T const & b) {
    Vector<T,N+1> r;
    std::copy(a.begin(),a.end(),r.begin());
    r[N] = b;
    return r;
}

/// @brief Return a new Vector with the given scalar prepended to the original.
template <typename T, int N>
inline Vector<T,N+1> concatenate(T const & a, Vector<T,N> const & b) {
    Vector<T,N+1> r;
    r[0] = a;
    std::copy(b.begin(),b.end(),r.begin()+1);
    return r;
}

#ifndef DOXYGEN
BOOST_PP_REPEAT_FROM_TO(1, NDARRAY_MAKE_VECTOR_MAX, NDARRAY_MAKE_VECTOR_SPEC, unused)
#else
/**
 *  @brief Variadic constructor for Vector. 
 *
 *  Defined for N in [0 - NDARRAY_MAKE_VECTOR_MAX).
 */
template <typename T, int N>
Vector<T,N> makeVector(T v1, T v2, ..., T vN);
#endif

/** @brief Unary bitwise NOT for Vector. */
template <typename T, int N>
inline Vector<T,N> operator~(Vector<T,N> const & vector) {
    Vector<T,N> r(vector);
    for (typename Vector<T,N>::Iterator i = r.begin(); i != r.end(); ++i) (*i) = ~(*i);
    return r;    
}

/** @brief Unary negation for Vector. */
template <typename T, int N>
inline Vector<T,N> operator!(Vector<T,N> const & vector) {
    Vector<T,N> r(vector);
    for (typename Vector<T,N>::Iterator i = r.begin(); i != r.end(); ++i) (*i) = !(*i);
    return r;
}

VECTOR_BINARY_OP(+)
VECTOR_BINARY_OP(-)
VECTOR_BINARY_OP(*)
VECTOR_BINARY_OP(/)
VECTOR_BINARY_OP(%)
VECTOR_BINARY_OP(&)
VECTOR_BINARY_OP(^)
VECTOR_BINARY_OP(|)
VECTOR_BINARY_OP(<<)
VECTOR_BINARY_OP(>>)

/// @}

} // namespace ndarray

#endif // !NDARRAY_Vector_h_INCLUDED
