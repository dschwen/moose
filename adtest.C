#include <iostream>

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
  const T _value;
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
  const auto eval() const { return this->_ref[_idx]; }

protected:
  const I & _idx;
};

template <typename L, typename R>
class EPAdd : public EPBase
{
public:
  EPAdd(L left, R right) : _left(left), _right(right) {}
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

  typedef typename EPSuperType<typename L::O, typename R::O>::type O;

protected:
  const L _left;
  const R _right;
};

template <typename L, typename R>
class EPMul : public EPBase
{
public:
  EPMul(L left, R right) : _left(left), _right(right) {}
  const auto eval() const
  {
    if constexpr (std::is_base_of<EPNullBase, L>::value || std::is_base_of<EPNullBase, R>::value)
      return O(0);

    if constexpr (std::is_base_of<EPOneBase, L>::value && std::is_base_of<EPOneBase, R>::value)
      return EPOne<O>();

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

  typedef typename EPSuperType<typename L::O, typename R::O>::type O;

protected:
  const L _left;
  const R _right;
};

template <typename L, typename R
          // ,
          // class = std::enable_if_t<std::is_base_of<EPBase, L>::value &&
          //                          std::is_base_of<EPBase, R>::value>
          >
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

int
main()
{
  double x = 1.0;
  const auto X = makeRef<1>(x);

  const auto result = 2 * (5 + X * X);
  const auto derivative = result.D<1>();

  x = 5.0;
  std::cout << result.eval() << ' ' << derivative.eval() << '\n';

  x = 3.0;
  std::cout << result.eval() << ' ' << derivative.eval() << '\n';

  return 0;
}
