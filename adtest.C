#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>

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

template <typename E>
auto exp(const E & exp);

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

  typedef T O;

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
class EPValue : public EPUnary<T>
{
public:
  EPValue(const T value) : EPUnary<T>(value) {}
  const auto eval() const { return _arg; }

  template <EPTag dtag>
  auto D() const
  {
    return EPNull<T>();
  }

  using EPUnary<T>::_arg;
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
    // MakeTree(cPow, a, b) * (D(b) * MakeTree(cLog, a) + b * D(a)/a);

    return pow(_left, _right) *
           _right.template D<dtag>(); // * log(_left) + _right * _left.D<dtag>()/_left;
  }

  using typename EPBinary<L, R>::O;
  using EPBinary<L, R>::_left;
  using EPBinary<L, R>::_right;
};

template <typename T>
class EPExp : public EPUnary<T>
{
public:
  EPExp(T arg) : EPUnary<T>(arg) {}
  const auto eval() const { return std::exp(_arg.eval()); }
  template <EPTag dtag>
  auto D() const
  {
    return exp(_arg);
  }

  using typename EPUnary<T>::O;
  using EPUnary<T>::_arg;
};

#define EP_OPERATOR_BINARY(op, OP)                                                                 \
  template <typename L, typename R>                                                                \
  auto operator op(const L & left, const R & right)                                                \
  {                                                                                                \
    if constexpr (std::is_base_of<EPBase, L>::value && std::is_base_of<EPBase, R>::value)          \
      return OP(left, right);                                                                      \
    else if constexpr (std::is_base_of<EPBase, L>::value)                                          \
      return OP(left, EPRef<0, R>(right));                                                         \
    else if constexpr (std::is_base_of<EPBase, R>::value)                                          \
      return OP(EPRef<0, L>(left), right);                                                         \
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

template <typename E>
auto
exp(const E & exp)
{
  return EPExp(exp);
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

int
main()
{
  double x;

  {
    double a = 0;
    const auto X = makeRef<1>(x);
    const auto result = X * (1.0 + X * (3.0 - X * (2.0 + X * (5.0 - X))));
    auto a1 = std::chrono::system_clock::now();
    for (x = -10; x < 10; x += 1e-6)
      a += result.eval();
    auto a2 = std::chrono::system_clock::now();
    std::cout << a << ' ' << std::chrono::duration_cast<std::chrono::milliseconds>(a2 - a1).count()
              << std::endl;
  }

  {
    double b = 0;
    const auto & X = x;
    auto b1 = std::chrono::system_clock::now();
    for (x = -10; x < 10; x += 1e-6)
      b += X * (1.0 + X * (3.0 - X * (2.0 + X * (5.0 - X))));
    auto b2 = std::chrono::system_clock::now();

    std::cout << b << ' ' << std::chrono::duration_cast<std::chrono::milliseconds>(b2 - b1).count()
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
