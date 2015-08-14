#ifndef TYPETUPEL_H
#define TYPETUPEL_H

#include "MooseTypes.h"

template <typename T, int N>
class TypeTupel;

template <int N> class RealTupel : public TypeTupel<Real, N> {};

template <typename T, int N>
class TypeTupel {
public:
  TypeTupel();

  T & operator[] (unsigned i) { return data[i]; }
  const T & operator[] (unsigned i) const { return data[i]; }

  bool operator< (const TypeTupel<T,N> & rhs);
  bool operator>= (const TypeTupel<T,N> & rhs);
  bool operator> (const TypeTupel<T,N> & rhs);
  bool operator<= (const TypeTupel<T,N> & rhs);
  bool operator== (const TypeTupel<T,N> & rhs);
  bool operator!= (const TypeTupel<T,N> & rhs);

private:
  T data[N];
};

template <typename T, int N>
bool
TypeTupel<T,N>::operator< (const TypeTupel<T,N> & rhs)
{
  for (unsigned int i = 0; i < N; ++i)
    if (data[i] < rhs.data[i])
      return true;
  return false;
}

template <typename T, int N>
bool
TypeTupel<T,N>::operator>= (const TypeTupel<T,N> & rhs)
{
  return !(data[i] < rhs.data[i]);
}

template <typename T, int N>
bool
TypeTupel<T,N>::operator< (const TypeTupel<T,N> & rhs)
{
  for (unsigned int i = 0; i < N; ++i)
    if (data[i] != rhs.data[i])
      return false;
  return true;
}

template <typename T, int N>
bool
TypeTupel<T,N>::operator!= (const TypeTupel<T,N> & rhs)
{
  return !(data[i] == rhs.data[i]);
}

#endif //TYPETUPEL_H
