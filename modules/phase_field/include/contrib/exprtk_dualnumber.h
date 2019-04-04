//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// DualNumber specializations and overloads for exprtk

#ifndef EXPRTK_DUALREAL_ADAPTOR_HPP
#define EXPRTK_DUALREAL_ADAPTOR_HPP

namespace exprtk
{
namespace details
{
namespace numeric
{
namespace details
{
struct dualnumber_type_tag;

template <typename T>
inline T const_pi_impl(dualnumber_type_tag);
template <typename T>
inline T const_e_impl(dualnumber_type_tag);
}
}

template <typename T, typename U>
inline bool is_true(const DualNumber<T, U> v);
template <typename T, typename U>
inline bool is_false(const DualNumber<T, U> v);

template <typename Iterator, typename T, typename U>
inline bool string_to_real(Iterator & itr_external,
                           const Iterator end,
                           DualNumber<T, U> & t,
                           details::numeric::details::dualnumber_type_tag);
}

namespace rtl
{
namespace io
{
namespace details
{
template <typename T, typename U>
inline void print_type(const std::string & fmt,
                       const DualNumber<T, U> & v,
                       exprtk::details::numeric::details::dualnumber_type_tag);
}
}
}

using details::is_true;
}

#include "exprtk.h"

namespace exprtk
{
namespace details
{
namespace numeric
{
namespace details
{
struct dualnumber_type_tag
{
};

template <typename T, typename U>
struct number_type<DualNumber<T, U>>
{
  typedef dualnumber_type_tag type;
};

template <>
struct epsilon_type<dualnumber_type_tag>
{
  static inline Real value()
  {
    const Real epsilon = 0.000000000001;
    return epsilon;
  }
};

template <typename T, typename U>
inline bool
is_nan_impl(const DualNumber<T, U> & v, dualnumber_type_tag)
{
  return MetaPhysicL::raw_value(v) != MetaPhysicL::raw_value(v);
}

template <typename T>
inline T
abs_impl(const T v, dualnumber_type_tag)
{
  return std::abs(v);
}
template <typename T>
inline T
acos_impl(const T v, dualnumber_type_tag)
{
  return std::acos(v);
}
template <typename T>
inline T
acosh_impl(const T v, dualnumber_type_tag)
{
  return std::log(v + std::sqrt((v * v) - T(1)));
}
template <typename T>
inline T
asin_impl(const T v, dualnumber_type_tag)
{
  return std::asin(v);
}
template <typename T>
inline T
asinh_impl(const T v, dualnumber_type_tag)
{
  return std::log(v + std::sqrt((v * v) + T(1)));
}
template <typename T>
inline T
atan_impl(const T v, dualnumber_type_tag)
{
  return std::atan(v);
}
template <typename T>
inline T
atanh_impl(const T v, dualnumber_type_tag)
{
  return (std::log(T(1) + v) - std::log(T(1) - v)) / T(2);
}
template <typename T>
inline T
ceil_impl(const T v, dualnumber_type_tag)
{
  return std::ceil(v);
}
template <typename T>
inline T
cos_impl(const T v, dualnumber_type_tag)
{
  return std::cos(v);
}
template <typename T>
inline T
cosh_impl(const T v, dualnumber_type_tag)
{
  return std::cosh(v);
}
template <typename T>
inline T
exp_impl(const T v, dualnumber_type_tag)
{
  return std::exp(v);
}
template <typename T>
inline T
floor_impl(const T v, dualnumber_type_tag)
{
  return std::floor(v);
}
template <typename T>
inline T
log_impl(const T v, dualnumber_type_tag)
{
  return std::log(v);
}
template <typename T>
inline T
log10_impl(const T v, dualnumber_type_tag)
{
  return std::log10(v);
}
template <typename T>
inline T
log2_impl(const T v, dualnumber_type_tag)
{
  return std::log(v) / numeric::constant::log2;
}
template <typename T>
inline T
neg_impl(const T v, dualnumber_type_tag)
{
  return -v;
}
template <typename T>
inline T
pos_impl(const T v, dualnumber_type_tag)
{
  return v;
}
template <typename T>
inline T
sin_impl(const T v, dualnumber_type_tag)
{
  return std::sin(v);
}
template <typename T>
inline T
sinh_impl(const T v, dualnumber_type_tag)
{
  return std::sinh(v);
}
template <typename T>
inline T
sqrt_impl(const T v, dualnumber_type_tag)
{
  return std::sqrt(v);
}
template <typename T>
inline T
tan_impl(const T v, dualnumber_type_tag)
{
  return std::tan(v);
}
template <typename T>
inline T
tanh_impl(const T v, dualnumber_type_tag)
{
  return std::tanh(v);
}
template <typename T>
inline T
cot_impl(const T v, dualnumber_type_tag)
{
  return 1.0 / std::tan(v);
}
template <typename T>
inline T
sec_impl(const T v, dualnumber_type_tag)
{
  return 1.0 / std::cos(v);
}
template <typename T>
inline T
csc_impl(const T v, dualnumber_type_tag)
{
  return 1.0 / std::sin(v);
}
template <typename T>
inline T
r2d_impl(const T v, dualnumber_type_tag)
{
  return (v * numeric::constant::_180_pi);
}
template <typename T>
inline T
d2r_impl(const T v, dualnumber_type_tag)
{
  return (v * numeric::constant::pi_180);
}
template <typename T>
inline T
d2g_impl(const T v, dualnumber_type_tag)
{
  return (v * T(20.0 / 9.0));
}
template <typename T>
inline T
g2d_impl(const T v, dualnumber_type_tag)
{
  return (v * T(9.0 / 20.0));
}
template <typename T>
inline T
notl_impl(const T v, dualnumber_type_tag)
{
  return (v != 0.0 ? T(0) : T(1));
}
template <typename T>
inline T
frac_impl(const T v, dualnumber_type_tag)
{
  return (v - std::trunc(v));
}
template <typename T>
inline T
trunc_impl(const T v, dualnumber_type_tag)
{
  return std::trunc(v);
}

template <typename T>
inline T const_pi_impl(dualnumber_type_tag)
{
  return numeric::constant::pi;
}
template <typename T>
inline T const_e_impl(dualnumber_type_tag)
{
  return numeric::constant::e;
}

template <typename T>
inline int
to_int32_impl(const T v, dualnumber_type_tag)
{
  return static_cast<int>(MetaPhysicL::raw_value(v));
}

template <typename T>
inline long long
to_int64_impl(const T v, dualnumber_type_tag)
{
  return static_cast<long long int>(MetaPhysicL::raw_value(v));
}

template <typename T, typename U>
inline bool
is_true_impl(const DualNumber<T, U> v)
{
  return (0.0 != v);
}

template <typename T, typename U>
inline bool
is_false_impl(const DualNumber<T, U> v)
{
  return (0.0 == v);
}

template <typename T>
inline T
expm1_impl(const T v, dualnumber_type_tag)
{
  if (std::abs(MetaPhysicL::raw_value(v)) < 0.00001)
    return v + (0.5 * v * v);
  else
    return std::exp(v) - 1.0;
}

template <typename T>
inline T
nequal_impl(const T v0, const T v1, dualnumber_type_tag)
{
  const T epsilon = epsilon_type<T>::value();
  const T eps_norm =
      (std::max(T(1),
                std::max(abs_impl(v0, dualnumber_type_tag()), abs_impl(v1, dualnumber_type_tag()))) *
       epsilon);
  return (abs_impl(v0 - v1, dualnumber_type_tag()) > eps_norm) ? 1.0 : 0.0;
}

template <typename T>
inline T
sgn_impl(const T v, dualnumber_type_tag)
{
  if (MetaPhysicL::raw_value(v) > 0)
    return 1.0;
  else if (MetaPhysicL::raw_value(v) < 0)
    return -1.0;
  else
    return 0.0;
}

template <typename T>
inline T
log1p_impl(const T v, dualnumber_type_tag)
{
  if (MetaPhysicL::raw_value(v) > -1.0)
  {
    if (abs_impl(v, dualnumber_type_tag()) > T(0.0001))
    {
      return log_impl(T(1) + v, dualnumber_type_tag());
    }
    else
      return (T(-0.5) * v + T(1)) * v;
  }
  else
    return T(std::numeric_limits<T>::quiet_NaN());
}

template <typename T>
inline T
erf_impl(T v, dualnumber_type_tag)
{
  const T t = T(1) / (T(1) + T(0.5) * abs_impl(v, dualnumber_type_tag()));

  static const T c[] = {T(1.26551223),
                        T(1.00002368),
                        T(0.37409196),
                        T(0.09678418),
                        T(-0.18628806),
                        T(0.27886807),
                        T(-1.13520398),
                        T(1.48851587),
                        T(-0.82215223),
                        T(0.17087277)};

  T result =
      T(1) -
      t * exp_impl(
              (-v * v) - c[0] +
                  t * (c[1] +
                       t * (c[2] +
                            t * (c[3] +
                                 t * (c[4] +
                                      t * (c[5] +
                                           t * (c[6] + t * (c[7] + t * (c[8] + t * (c[9]))))))))),
              dualnumber_type_tag());

  return (v >= T(0)) ? result : -result;
}

template <typename T>
inline T
erfc_impl(T v, dualnumber_type_tag)
{
  return T(1) - erf_impl(v, dualnumber_type_tag());
}

template <typename T>
inline T
ncdf_impl(T v, dualnumber_type_tag)
{
  T cnd = 0.5 * (1.0 + erf_impl(abs_impl(v, dualnumber_type_tag()) / T(numeric::constant::sqrt2),
                                dualnumber_type_tag()));
  return (v < 0.0) ? (1.0 - cnd) : cnd;
}

template <typename T>
inline T
modulus_impl(const T v0, const T v1, dualnumber_type_tag)
{
  return std::fmod(v0, v1);
}

template <typename T>
inline T
modulus_impl(const T v0, const double v1, dualnumber_type_tag)
{
  return std::fmod(v0, v1);
}

template <typename T>
inline T
pow_impl(const T v0, const T v1, dualnumber_type_tag)
{
  return std::pow(v0, v1);
}

template <typename T>
inline T
logn_impl(const T v0, const T v1, dualnumber_type_tag)
{
  return log(v0) / log(v1);
}

template <typename T>
inline T
sinc_impl(T v, dualnumber_type_tag)
{
  if (abs_impl(v, dualnumber_type_tag()) >= std::numeric_limits<T>::epsilon())
    return (sin_impl(v, dualnumber_type_tag()) / v);
  else
    return T(1);
}

template <typename T>
inline T
xor_impl(const T v0, const T v1, dualnumber_type_tag)
{
  return (is_false_impl(v0) != is_false_impl(v1)) ? 1.0 : 0.0;
}

template <typename T>
inline T
xnor_impl(const T v0, const T v1, dualnumber_type_tag)
{
  const bool v0_true = is_true_impl(v0);
  const bool v1_true = is_true_impl(v1);
  if ((v0_true && v1_true) || (!v0_true && !v1_true))
    return T(1);
  else
    return T(0);
}

template <typename T>
inline T
equal_impl(const T v0, const T v1, dualnumber_type_tag)
{
  const T epsilon = epsilon_type<T>::value();
  const T eps_norm =
      (std::max(T(1),
                std::max(abs_impl(v0, dualnumber_type_tag()), abs_impl(v1, dualnumber_type_tag()))) *
       epsilon);
  return (abs_impl(v0 - v1, dualnumber_type_tag()) <= eps_norm) ? 1.0 : 0.0;
}

template <typename T>
inline T
round_impl(const T v, dualnumber_type_tag)
{
  return ((v < T(0)) ? std::ceil(v - 0.5) : std::floor(v + 0.5));
}

template <typename T>
inline T
roundn_impl(const T v0, const T v1, dualnumber_type_tag)
{
  const int index =
      std::max<int>(0, std::min<int>(pow10_size - 1, std::floor(MetaPhysicL::raw_value(v1))));
  const T p10 = T(pow10[index]);
  if (v0 < T(0))
    return T(std::ceil((v0 * p10) - 0.5) / p10);
  else
    return T(std::floor((v0 * p10) + 0.5) / p10);
}

template <typename T>
inline bool
is_integer_impl(const T v, dualnumber_type_tag)
{
  return (T(0) == modulus_impl(v, 1.0, dualnumber_type_tag()));
}

template <typename T>
inline T
root_impl(const T v0, const T v1, dualnumber_type_tag)
{
  return std::pow(v0, 1.0 / v1);
}

template <typename T>
inline T
hypot_impl(const T v0, const T v1, dualnumber_type_tag)
{
  return sqrt((v0 * v0) + (v1 * v1));
}

template <typename T>
inline T
atan2_impl(const T v0, const T v1, dualnumber_type_tag)
{
  return std::atan2(v0, v1);
}

template <typename T>
inline T
shr_impl(const T v0, const T v1, dualnumber_type_tag)
{
  return v0 * (T(1) / std::pow(T(2), v1));
}

template <typename T>
inline T
shl_impl(const T v0, const T v1, dualnumber_type_tag)
{
  return v0 * std::pow(T(2), v1);
}

template <typename T>
inline T
and_impl(const T v0, const T v1, dualnumber_type_tag)
{
  return (is_true_impl(v0) && is_true_impl(v1)) ? 1.0 : 0.0;
}

template <typename T>
inline T
nand_impl(const T v0, const T v1, dualnumber_type_tag)
{
  return (is_false_impl(v0) || is_false_impl(v1)) ? 1.0 : 0.0;
}

template <typename T>
inline T
or_impl(const T v0, const T v1, dualnumber_type_tag)
{
  return (is_true_impl(v0) || is_true_impl(v1)) ? 1.0 : 0.0;
}

template <typename T>
inline T
nor_impl(const T v0, const T v1, dualnumber_type_tag)
{
  return (is_false_impl(v0) && is_false_impl(v1)) ? 1.0 : 0.0;
}
}
}

template <typename Iterator, typename T, typename U>
inline bool
string_to_real(Iterator & itr_external,
               const Iterator end,
               DualNumber<T, U> & t,
               details::numeric::details::dualnumber_type_tag)
{
  typename numeric::details::number_type<double>::type num_type;
  Real dummy;
  bool r = string_to_real<Iterator, double>(itr_external, end, dummy, num_type);
  t = dummy;
  return r;
}

template <typename T, typename U>
inline bool
is_true(const DualNumber<T, U> v)
{
  return MetaPhysicL::raw_value(v) != 0.0;
}
template <typename T, typename U>
inline bool
is_false(const DualNumber<T, U> v)
{
  return MetaPhysicL::raw_value(v) == 0.0;
}
}

namespace rtl
{
namespace io
{
namespace details
{
template <typename T, typename U>
inline void
print_type(const std::string & fmt,
           const DualNumber<T, U> v,
           exprtk::details::numeric::details::dualnumber_type_tag)
{
  printf(fmt.c_str(), MetaPhysicL::raw_value(v));
}
}
}
}
}

#endif
