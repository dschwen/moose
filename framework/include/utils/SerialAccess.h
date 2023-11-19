//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Moose.h"
#include "MooseTypes.h"

#include <tuple>

namespace Moose
{

/**
 * Serial access requires object data to be stored contiguously. Specialize this template
 * to support more types.
 */
template <typename T>
struct SerialAccess
{
  static_assert(always_false<T>, "Specialize SerialAccess for this type.");
};

/// simple Real specialization
template <>
struct SerialAccess<Real>
{
  static Real * data(Real & obj) { return &obj; }
  static constexpr std::size_t size(Real &) { return 1u; }
  static constexpr std::size_t size() { return 1u; }
};
template <>
struct SerialAccess<ADReal>
{
  static ADReal * data(ADReal & obj) { return &obj; }
  static constexpr std::size_t size(ADReal &) { return 1u; }
  static constexpr std::size_t size() { return 1u; }
};

/// (AD)RealVectorValue etc. specialization
template <typename T>
struct SerialAccess<VectorValue<T>>
{
  static T * data(VectorValue<T> & obj) { return &obj(0u); }
  static constexpr std::size_t size(VectorValue<T> &) { return Moose::dim; }
  static constexpr std::size_t size() { return Moose::dim; }
};

/// DenseVector specialization
template <typename T>
struct SerialAccess<DenseVector<T>>
{
  static T * data(DenseVector<T> & obj) { return &obj(0u); }
  static std::size_t size(DenseVector<T> & obj) { return obj.size(); }
};

/**
 * Value type helper (necessary for any type that does not have a value_type
 * member or where value_type doesn't have a suitable meaning (ADReal)).
 */
template <typename T>
struct SerialAccessVlaueTypeHelper
{
  typedef typename T::value_type value_type;
};
template <>
struct SerialAccessVlaueTypeHelper<ADReal>
{
  typedef ADReal value_type;
};
template <>
struct SerialAccessVlaueTypeHelper<Real>
{
  typedef Real value_type;
};
template <>
struct SerialAccessVlaueTypeHelper<const Real>
{
  typedef const Real value_type;
};

template <typename T>
class SerialAccessRange
{
  typedef typename SerialAccessVlaueTypeHelper<T>::value_type V;

public:
  class iterator
  {
  public:
    iterator(V * i) : _i(i) {}

    V & operator*() const { return *_i; }

    const iterator & operator++()
    {
      ++_i;
      return *this;
    }

    iterator operator++(int)
    {
      iterator returnval(*this);
      ++_i;
      return returnval;
    }

    bool operator==(const iterator & j) const { return (_i == j._i); }
    bool operator!=(const iterator & j) const { return !(*this == j); }

  private:
    V * _i;
  };

  SerialAccessRange(T & obj)
    : _begin(SerialAccess<T>::data(obj)),
      _end(SerialAccess<T>::data(obj) + SerialAccess<T>::size(obj))
  {
  }

  iterator begin() const { return _begin; }

  iterator end() const { return _end; }

private:
  iterator _begin, _end;
};

template <typename T>
SerialAccessRange<T>
serialAccess(T & obj)
{
  return SerialAccessRange<T>(obj);
}

/// Helper structure to hold a list of types
template <typename... Ts>
struct TypeList
{
  typedef std::tuple<Ts...> Tuple;
  typedef std::tuple<Ts *...> PointerTuple;
  static constexpr std::size_t size = sizeof...(Ts);
};

/// Type loop
template <template <typename, int> class L, int I, typename T, typename... Ts, typename... As>
void
typeLoopInternal(TypeList<T, Ts...>, As... args)
{
  L<T, I>::apply(args...);
  if constexpr (sizeof...(Ts) > 0)
    typeLoopInternal<L, I + 1>(TypeList<Ts...>{}, args...);
}

/// Type loop
template <template <typename, int> class L, typename... Ts, typename... As>
void
typeLoop(TypeList<Ts...>, As... args)
{
  typeLoopInternal<L, 0>(TypeList<Ts...>{}, args...);
}

} // namespace Moose;
