#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

/////
template <int N, typename T>
struct do_pow
{
  static inline T apply(const T & x)
  {
    if constexpr (N % 2) // odd exponent
      return x * do_pow<N - 1, T>::apply(x);

    const T xNover2 = do_pow<N / 2, T>::apply(x);
    return xNover2 * xNover2;
  }
};

template <typename T>
struct do_pow<6, T>
{
  static inline T apply(const T & x)
  {
    const T x2 = x * x, x4 = x2 * x2;
    return x4 * x2;
  }
};

template <typename T>
struct do_pow<1, T>
{
  static inline T apply(const T & x) { return x; }
};

template <typename T>
struct do_pow<0, T>
{
  static inline T apply(const T &) { return 1; }
};
/////

class EPBase
{
};
class EPNullBase : public EPBase
{
};
class EPOneBase : public EPBase
{
};

template <typename T1, typename T2>
struct EPSuperType
{
  typedef double type;
};

using EPTag = int;

template <typename L, typename R
          // ,
          // class = std::enable_if_t<std::is_base_of<EPBase, L>::value &&
          //                          std::is_base_of<EPBase, R>::value>
          >
auto operator+(const L & left, const R & right);

template <typename L, typename R
          // ,
          // class = std::enable_if_t<std::is_base_of<EPBase, L>::value &&
          //                          std::is_base_of<EPBase, R>::value>
          >
auto operator*(const L & left, const R & right);

template <typename B, typename E>
auto pow(const B & base, const E & exp);
template <int E, typename B>
auto pow(const B &);

template <typename T>
auto exp(const T &);
template <typename T>
auto log(const T &);

template <typename T>
class EPNull : public EPNullBase
{
public:
  EPNull() {}
  const auto eval() const { return T(0); }

  template <EPTag dtag>
  auto D() const
  {
    return EPNull<T>();
  }

  typedef T O;
};

template <typename T>
class EPUnary : public EPBase
{
public:
  EPUnary(T arg) : _arg(arg) {}

  typedef typename T::O O;

protected:
  const T _arg;
};

template <typename L, typename R>
class EPBinary : public EPBase
{
public:
  EPBinary(L left, R right) : _left(left), _right(right) {}

  typedef typename EPSuperType<typename L::O, typename R::O>::type O;

protected:
  const L _left;
  const R _right;
};

template <typename T>
class EPOne : public EPOneBase
{
public:
  EPOne() {}
  const auto eval() const { return T(1); }

  template <EPTag dtag>
  auto D() const
  {
    return EPNull<T>();
  }

  typedef T O;
};

template <typename T>
class EPValue : public EPBase
{
public:
  EPValue(const T value) : _value(value) {}
  const auto eval() const { return _value; }

  template <EPTag dtag>
  auto D() const
  {
    return EPNull<T>();
  }

  typedef T O;

protected:
  T _value;
};

template <EPTag tag, typename T>
class EPRef : public EPBase
{
public:
  EPRef(const T & ref) : _ref(ref) {}
  const auto eval() const { return _ref; }

  template <EPTag dtag>
  auto D() const
  {
    if constexpr (tag == dtag)
      return EPOne<T>();
    else
      return EPNull<T>();
  }

  typedef T O;

protected:
  const T & _ref;
};

template <EPTag tag, typename T, typename I>
class EPArrayRef : public EPRef<tag, T>
{
public:
  EPArrayRef(const T & ref, const I & idx) : EPRef<tag, T>(ref), _idx(idx) {}
  const auto eval() const { return _ref[_idx]; }

  template <EPTag dtag>
  auto D() const
  {
    if constexpr (tag == dtag)
      return EPOne<typename T::value_type>();
    else
      return EPNull<typename T::value_type>();
  }

  typedef typename T::value_type O;
  using EPRef<tag, T>::_ref;

protected:
  const I & _idx;
};

template <EPTag tag, typename T, typename I>
class EPArrayValue : public EPRef<tag, T>
{
public:
  EPArrayValue(I idx) : _idx(idx) {}
  const auto eval() const { return _ref[_idx]; }

  using typename EPUnary<T>::O;
  using EPUnary<T>::_ref;

protected:
  const I _idx;
};

template <typename L, typename R>
class EPAdd : public EPBinary<L, R>
{
public:
  EPAdd(L left, R right) : EPBinary<L, R>(left, right) {}
  const auto eval() const
  {
    if constexpr (std::is_base_of<EPNullBase, L>::value && std::is_base_of<EPNullBase, R>::value)
      return O(0);

    if constexpr (std::is_base_of<EPNullBase, L>::value)
      return _right.eval();

    if constexpr (std::is_base_of<EPNullBase, R>::value)
      return _left.eval();

    else
      return _left.eval() + _right.eval();
  }

  template <EPTag dtag>
  auto D() const
  {
    return _left.template D<dtag>() + _right.template D<dtag>();
  }

  using typename EPBinary<L, R>::O;
  using EPBinary<L, R>::_left;
  using EPBinary<L, R>::_right;
};

template <typename L, typename R>
class EPSub : public EPBinary<L, R>
{
public:
  EPSub(L left, R right) : EPBinary<L, R>(left, right) {}
  const auto eval() const
  {
    if constexpr (std::is_base_of<EPNullBase, L>::value && std::is_base_of<EPNullBase, R>::value)
      return O(0);

    if constexpr (std::is_base_of<EPNullBase, L>::value)
      return -_right.eval();

    if constexpr (std::is_base_of<EPNullBase, R>::value)
      return _left.eval();

    else
      return _left.eval() - _right.eval();
  }

  template <EPTag dtag>
  auto D() const
  {
    return _left.template D<dtag>() - _right.template D<dtag>();
  }

  using typename EPBinary<L, R>::O;
  using EPBinary<L, R>::_left;
  using EPBinary<L, R>::_right;
};

template <typename L, typename R>
class EPMul : public EPBinary<L, R>
{
public:
  EPMul(L left, R right) : EPBinary<L, R>(left, right) {}
  const auto eval() const
  {
    if constexpr (std::is_base_of<EPNullBase, L>::value || std::is_base_of<EPNullBase, R>::value)
      return O(0);

    if constexpr (std::is_base_of<EPOneBase, L>::value && std::is_base_of<EPOneBase, R>::value)
      return O(1);

    if constexpr (std::is_base_of<EPOneBase, L>::value)
      return _right.eval();

    if constexpr (std::is_base_of<EPOneBase, R>::value)
      return _left.eval();

    else
      return _left.eval() * _right.eval();
  }

  template <EPTag dtag>
  auto D() const
  {
    return _left.template D<dtag>() * _right + _right.template D<dtag>() * _left;
  }

  using typename EPBinary<L, R>::O;
  using EPBinary<L, R>::_left;
  using EPBinary<L, R>::_right;
};

template <typename L, typename R>
class EPDiv : public EPBinary<L, R>
{
public:
  EPDiv(L left, R right) : EPBinary<L, R>(left, right) {}
  const auto eval() const
  {
    if constexpr (std::is_base_of<EPOneBase, R>::value)
      return _left.eval();

    if constexpr (std::is_base_of<EPNullBase, L>::value && !std::is_base_of<EPNullBase, R>::value)
      return O(0);

    return _left.eval() / _right.eval();
  }

  template <EPTag dtag>
  auto D() const
  {
    return _left.template D<dtag>() / _right - _right.template D<dtag>() / (_right * _right);
  }

  using typename EPBinary<L, R>::O;
  using EPBinary<L, R>::_left;
  using EPBinary<L, R>::_right;
};

template <typename L, typename R>
class EPPow : public EPBinary<L, R>
{
public:
  EPPow(L left, R right) : EPBinary<L, R>(left, right) {}
  const auto eval() const
  {
    if constexpr (std::is_base_of<EPNullBase, L>::value)
      return O(0);

    if constexpr (std::is_base_of<EPOneBase, L>::value || std::is_base_of<EPNullBase, R>::value)
      return O(1);

    if constexpr (std::is_base_of<EPOneBase, R>::value)
      return _left.eval();

    return std::pow(_left.eval(), _right.eval());
  }

  template <EPTag dtag>
  auto D() const
  {
    return pow(_left, _right) * _right.template D<dtag>() * log(_left) +
           _right * _left.template D<dtag>() / _left;
  }

  using typename EPBinary<L, R>::O;
  using EPBinary<L, R>::_left;
  using EPBinary<L, R>::_right;
};

template <typename B, int E>
class EPIPow : public EPUnary<B>
{
public:
  EPIPow(B base) : EPUnary<B>(base) {}
  const auto eval() const
  {
    if constexpr (std::is_base_of<EPNullBase, B>::value)
      return O(0);

    else if constexpr (std::is_base_of<EPOneBase, B>::value || E == 0)
      return O(1);

    else if constexpr (E == 1)
      return _arg.eval();

    // replace with Utility::pow !
    else if constexpr (E < 0)
      return 1.0 / do_pow<-E, O>::apply(_arg.eval());

    else
      return do_pow<E, O>::apply(_arg.eval());
  }

  template <EPTag dtag>
  auto D() const
  {
    if constexpr (E == 1)
      return _arg.template D<dtag>();

    else if constexpr (E == 0)
      return EPNull<O>();

    else
      return pow<E - 1>(_arg) * E * _arg.template D<dtag>();
  }

  using typename EPUnary<B>::O;
  using EPUnary<B>::_arg;
};

#define EP_OPERATOR_BINARY(op, OP)                                                                 \
  template <typename L, typename R>                                                                \
  auto operator op(const L & left, const R & right)                                                \
  {                                                                                                \
    if constexpr (std::is_base_of<EPBase, L>::value && std::is_base_of<EPBase, R>::value)          \
      return OP(left, right);                                                                      \
    else if constexpr (std::is_base_of<EPBase, L>::value)                                          \
      return OP(left, EPRef<-1, R>(right));                                                        \
    else if constexpr (std::is_base_of<EPBase, R>::value)                                          \
      return OP(EPRef<-1, L>(left), right);                                                        \
  }                                                                                                \
  template <typename L, typename R, class = std::enable_if_t<!std::is_base_of<EPBase, L>::value>>  \
  auto operator op(const L && left, const R & right)                                               \
  {                                                                                                \
    return OP(EPValue<L>(left), right);                                                            \
  }                                                                                                \
  template <typename L, typename R, class = std::enable_if_t<!std::is_base_of<EPBase, R>::value>>  \
  auto operator op(const L & left, const R && right)                                               \
  {                                                                                                \
    return OP(left, EPValue<R>(right));                                                            \
  }

EP_OPERATOR_BINARY(+, EPAdd)
EP_OPERATOR_BINARY(-, EPSub)
EP_OPERATOR_BINARY(*, EPMul)
EP_OPERATOR_BINARY(/, EPDiv)

#define EP_SIMPLE_UNARY_FUNCTION(name, eval_term, derivative)                                      \
  template <typename T>                                                                            \
  class EPF##name : public EPUnary<T>                                                              \
  {                                                                                                \
  public:                                                                                          \
    EPF##name(T arg) : EPUnary<T>(arg)                                                             \
    {                                                                                              \
    }                                                                                              \
    const auto eval() const                                                                        \
    {                                                                                              \
      return eval_term;                                                                            \
    }                                                                                              \
    template <EPTag dtag>                                                                          \
    auto D() const                                                                                 \
    {                                                                                              \
      return derivative;                                                                           \
    }                                                                                              \
    using typename EPUnary<T>::O;                                                                  \
    using EPUnary<T>::_arg;                                                                        \
  };                                                                                               \
  template <typename T>                                                                            \
  auto name(const T & v)                                                                           \
  {                                                                                                \
    return EPF##name(v);                                                                           \
  }

EP_SIMPLE_UNARY_FUNCTION(exp, std::exp(_arg.eval()), exp(_arg) * _arg.template D<dtag>())
EP_SIMPLE_UNARY_FUNCTION(log, std::log(_arg.eval()), _arg.template D<dtag>() / _arg)
EP_SIMPLE_UNARY_FUNCTION(sin, std::sin(_arg.eval()), cos(_arg) * _arg.template D<dtag>())
EP_SIMPLE_UNARY_FUNCTION(cos, std::cos(_arg.eval()), -1.0 * sin(_arg) * _arg.template D<dtag>())
EP_SIMPLE_UNARY_FUNCTION(tan,
                         std::tan(_arg.eval()),
                         (pow<2>(tan(_arg)) + 1.0) * _arg.template D<dtag>())
EP_SIMPLE_UNARY_FUNCTION(sqrt,
                         std::sqrt(_arg.eval()),
                         1.0 / (2.0 * sqrt(_arg)) * _arg.template D<dtag>())

template <EPTag tag, typename T>
auto
makeRef(const T & ref)
{
  return EPRef<tag, T>(ref);
}

template <EPTag tag, typename T, typename I>
auto
makeRef(const T & ref, const I & idx)
{
  return EPArrayRef<tag, T, I>(ref, idx);
}

template <EPTag tag, typename T, typename I>
auto
makeRef(const T & ref, const I && idx)
{
  return EPArrayValue<tag, T, I>(ref, idx);
}

template <typename B, typename E>
auto
pow(const B & base, const E & exp)
{
  return EPPow(base, exp);
}

template <int E, typename B>
auto
pow(const B & base)
{
  return EPIPow<B, E>(base);
}

// double
// time1(double x)
// {
//   const auto X = makeRef<1>(x);
//   const auto result = X * (1.0 + X * (3.0 - X * (2.0 + X * (5.0 - X))));
//   // double one = 1;
//   // const auto result = one + X + 1;
//   // one = 2;
//   return result.eval();
// }
//
// double
// time2(double x)
// {
//   const auto & X = x;
//   return X * (1.0 + X * (3.0 - X * (2.0 + X * (5.0 - X))));
// }

double
sqr(double a)
{
  return a * a;
}

int
main()
{
  double x;

  {
    enum
    {
      dX
    };

    const auto X = makeRef<dX>(x);
    const auto result = X * (1.0 - X) - (X * log(X) + (1.0 - X) * log(1.0 - X));

    double r0 = 0, r1 = 0, r2 = 0;
    auto a1 = std::chrono::system_clock::now();
    for (x = 0.01; x <= 0.99; x += 1e-6)
    {
      r0 += result.eval();
      r1 += result.D<dX>().eval();
      r2 += result.D<dX>().D<dX>().eval();
    }
    auto a2 = std::chrono::system_clock::now();

    std::cout << r0 << ' ' << r1 << ' ' << r2 << ' '
              << std::chrono::duration_cast<std::chrono::milliseconds>(a2 - a1).count()
              << std::endl;
  }

  {
    double r0 = 0, r1 = 0, r2 = 0;
    const auto & X = x;

    auto b1 = std::chrono::system_clock::now();
    for (x = 0.01; x <= 0.99; x += 1e-6)
    {
      r0 += X * (1.0 - X) - (X * log(X) + (1.0 - x) * log(1.0 - X));
      r1 += -2.0 * X - log(X) + log(1.0 - X) - (X - 1.0) / (1.0 - X);
      r2 += -2.0 + 1.0 / (X - 1.0) - 1.0 / X;
    }
    auto b2 = std::chrono::system_clock::now();

    std::cout << r0 << ' ' << r1 << ' ' << r2 << ' '
              << std::chrono::duration_cast<std::chrono::milliseconds>(b2 - b1).count()
              << std::endl;
  }

  // double x;
  //
  // const auto X = makeRef<1>(x);
  // const auto result = X * X + 100.0;
  //
  // x = 5;
  // std::cout << result.eval() << ' ' << result.D<1>().eval() << '\n';
  //
  // x = 3;
  // std::cout << result.eval() << ' ' << result.D<1>().eval() << '\n';

  // std::vector<double> _prop{1, 2, 3, 4};
  // long _qp;
  //
  // const auto prop = makeRef<1>(_prop, _qp);
  // const auto result = 3.0 * prop * prop;
  //
  // _qp = 1;
  // std::cout << result.eval() << ' ' << result.D<1>().eval() << '\n';
  //
  // _qp = 3;
  // std::cout << result.eval() << ' ' << result.D<1>().eval() << '\n';
  // std::cout << result.D<1>().D<1>().eval() << '\n';

  return 0;
}
