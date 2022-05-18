#include <iostream>
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
  EPArrayRef(const I & idx) : _idx(idx) {}
  const auto eval() const { return _ref[_idx]; }

  using typename EPUnary<T>::O;
  using EPUnary<T>::_ref;

protected:
  const I & _idx;
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

template <typename L, typename R>
auto
operator+(const L & left, const R & right)
{
  if constexpr (std::is_base_of<EPBase, L>::value && std::is_base_of<EPBase, R>::value)
    return EPAdd(left, right);
  else if constexpr (std::is_base_of<EPBase, L>::value)
    return EPAdd(left, EPRef<0, R>(right));
  else if constexpr (std::is_base_of<EPBase, R>::value)
    return EPAdd(EPRef<0, L>(left), right);
}

template <typename L, typename R, class = std::enable_if_t<!std::is_base_of<EPBase, L>::value>>
auto
operator+(const L && left, const R & right)
{
  return EPAdd(EPValue<L>(left), right);
}

template <typename L, typename R, class = std::enable_if_t<!std::is_base_of<EPBase, R>::value>>
auto
operator+(const L & left, const R && right)
{
  return EPAdd(left, EPValue<R>(right));
}

template <typename L, typename R
          // ,
          // class = std::enable_if_t<std::is_base_of<EPBase, L>::value &&
          //                          std::is_base_of<EPBase, R>::value>
          >
auto
operator-(const L & left, const R & right)
{
  if constexpr (std::is_base_of<EPBase, L>::value && std::is_base_of<EPBase, R>::value)
    return EPSub(left, right);
  else if constexpr (std::is_base_of<EPBase, L>::value)
  {
    if constexpr (std::is_lvalue_reference<R>::value)
      return EPSub(left, EPRef<0, R>(right));
    else
      return EPSub(left, EPValue<R>(right));
  }
  else if constexpr (std::is_base_of<EPBase, R>::value)
    return EPSub(EPRef<0, L>(left), right);
}

template <typename L, typename R, class = std::enable_if_t<!std::is_base_of<EPBase, L>::value>>
auto
operator-(const L && left, const R & right)
{
  return EPSub(EPValue<L>(left), right);
}

template <typename L, typename R, class = std::enable_if_t<!std::is_base_of<EPBase, R>::value>>
auto
operator-(const L & left, const R && right)
{
  return EPSub(left, EPValue<R>(right));
}

template <typename L, typename R
          // ,
          // class = std::enable_if_t<std::is_base_of<EPBase, L>::value &&
          //                          std::is_base_of<EPBase, R>::value>
          >
auto
operator*(const L & left, const R & right)
{
  if constexpr (std::is_base_of<EPBase, L>::value && std::is_base_of<EPBase, R>::value)
    return EPMul(left, right);
  else if constexpr (std::is_base_of<EPBase, L>::value)
    return EPMul(left, EPRef<0, R>(right));
  else if constexpr (std::is_base_of<EPBase, R>::value)
    return EPMul(EPRef<0, L>(left), right);
}

template <typename L, typename R, class = std::enable_if_t<!std::is_base_of<EPBase, L>::value>>
auto
operator*(const L && left, const R & right)
{
  return EPMul(EPValue<L>(left), right);
}

template <typename L, typename R, class = std::enable_if_t<!std::is_base_of<EPBase, R>::value>>
auto
operator*(const L & left, const R && right)
{
  return EPMul(left, EPValue<R>(right));
}

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

double
time1(double x)
{
  const auto X = makeRef<1>(x);
  const auto result = X * (1.0 + X * (3.0 - X * (2.0 + X * (5.0 - X))));
  // double one = 1;
  // const auto result = one + X + 1;
  // one = 2;
  return result.eval();
}

double
time2(double X)
{
  return X * (1.0 + X * (3.0 - X * (2.0 + X * (5.0 - X))));
}

int
main()
{
  double a = 0;
  auto a1 = std::chrono::system_clock::now();
  for (double x = -10; x < 10; x += 1e-6)
    a += time1(x);
  auto a2 = std::chrono::system_clock::now();
  std::cout << a << ' ' << std::chrono::duration_cast<std::chrono::milliseconds>(a2 - a1).count()
            << std::endl;

  double b = 0;
  auto b1 = std::chrono::system_clock::now();
  for (double x = -10; x < 10; x += 1e-6)
    b += time2(x);
  auto b2 = std::chrono::system_clock::now();

  std::cout << b << ' ' << std::chrono::duration_cast<std::chrono::milliseconds>(b2 - b1).count()
            << std::endl;

  return 0;
}
