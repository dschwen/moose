//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Moose.h"
#include "ADRankTwoTensorForward.h"
#include "ADSymmetricRankTwoTensorForward.h"
#include "ADSymmetricRankFourTensorForward.h"
#include "MooseUtils.h"

// Any requisite includes here
#include "libmesh/libmesh.h"
#include "libmesh/tensor_value.h"

#include "metaphysicl/raw_type.h"

#include <petscsys.h>
#include <vector>

// Forward declarations
class MooseEnum;
template <typename T>
class MooseArray;
typedef MooseArray<Real> VariableValue;
template <typename>
class ColumnMajorMatrixTempl;
namespace libMesh
{
template <typename>
class TypeVector;
template <typename>
class TypeTensor;
template <typename>
class TensorValue;
}

namespace MathUtils
{
template <typename T>
void mooseSetToZero(T & v);

/**
 * Helper function template specialization to set an object to zero.
 * Needed by DerivativeMaterialInterface
 */
template <>
void mooseSetToZero<SymmetricRankTwoTensor>(SymmetricRankTwoTensor & v);

/**
 * Helper function template specialization to set an object to zero.
 * Needed by DerivativeMaterialInterface
 */
template <>
void mooseSetToZero<ADSymmetricRankTwoTensor>(ADSymmetricRankTwoTensor & v);
}

/**
 * SymmetricRankTwoTensorTempl is designed to handle the Stress or Strain Tensor for
 * an anisotropic material. It is designed to reduce the redundancies of the
 * Complete tensor classes for regular mechanics problems and to enable use of the
 * Voigt notation.
 */
template <typename T>
class SymmetricRankTwoTensorTempl
{
public:
  // Select initialization
  enum InitMethod
  {
    initNone,
    initIdentity
  };

  /// Default constructor; fills to zero
  SymmetricRankTwoTensorTempl();

  /// Select specific initialization pattern
  SymmetricRankTwoTensorTempl(const InitMethod);

  /**
   * To fill up the 9 entries in the 2nd-order tensor, fillFromInputVector
   * is called with one of the following fill_methods.
   * See the fill*FromInputVector functions for more details
   */
  enum FillMethod
  {
    autodetect = 0,
    isotropic1 = 1,
    diagonal3 = 3,
    symmetric6 = 6
  };

  /**
   * Constructor that takes in 3 vectors and uses them to create rows
   * _vals[0][i] = row1(i), _vals[1][i] = row2(i), _vals[2][i] = row3(i)
   */
  SymmetricRankTwoTensorTempl(const TypeVector<T> & row1,
                              const TypeVector<T> & row2,
                              const TypeVector<T> & row3);

  /// Constructor that proxies the fillFromInputVector method
  SymmetricRankTwoTensorTempl(const std::vector<T> & input) { this->fillFromInputVector(input); };

  /// Initialization list replacement constructors, 6 arguments
  SymmetricRankTwoTensorTempl(T S11, T S22, T S33, T S23, T S13, T S12);

private:
  /// Initialization list replacement constructors, 9 arguments (for internal use only)
  SymmetricRankTwoTensorTempl(T S11, T S21, T S31, T S12, T S22, T S32, T S13, T S23, T S33);

public:
  /// Copy assignment operator must be defined if used
  SymmetricRankTwoTensorTempl(const SymmetricRankTwoTensorTempl<T> & a) = default;

  /// Copy constructor from TensorValue<T>
  explicit SymmetricRankTwoTensorTempl(const TensorValue<T> & a);

  /// Copy constructor from TypeTensor<T>
  explicit SymmetricRankTwoTensorTempl(const TypeTensor<T> & a);

  /// Construct from other template
  template <typename T2>
  SymmetricRankTwoTensorTempl(const SymmetricRankTwoTensorTempl<T2> & a)
  {
    for (std::size_t i = 0; i < N; ++i)
      _vals[i] = a(i);
  }

  // Named constructors
  static SymmetricRankTwoTensorTempl Identity()
  {
    return SymmetricRankTwoTensorTempl(initIdentity);
  }

  /// named constructor for initializing symmetrically
  static SymmetricRankTwoTensorTempl
  initializeSymmetric(const TypeVector<T> & v0, const TypeVector<T> & v1, const TypeVector<T> & v2);

  /// Static method for use in validParams for getting the "fill_method"
  static MooseEnum fillMethodEnum();

  /**
   * fillFromInputVector takes 6 or 9 inputs to fill in the Rank-2 tensor.
   * If 6 inputs, then symmetry is assumed S_ij = S_ji, and
   *   _vals[0][0] = input[0]
   *   _vals[1][1] = input[1]
   *   _vals[2][2] = input[2]
   *   _vals[1][2] = input[3]
   *   _vals[0][2] = input[4]
   *   _vals[0][1] = input[5]
   */
  void fillFromInputVector(const std::vector<T> & input, FillMethod fill_method = autodetect);

  /**
   * fillFromScalarVariable takes FIRST/THIRD/SIXTH order scalar variable to fill in the Rank-2
   * tensor.
   */
  void fillFromScalarVariable(const VariableValue & scalar_variable);

  /// Gets the value for the index specified.  Takes index = 0,1,2
  inline T & operator()(unsigned int i) { return _vals[i]; }

  /// multiply vector v with row n of this tensor
  T rowMultiply(std::size_t n, const TypeVector<T> & v) const;

  /// return the marix multiplied with its transpose A*A^T (guaranteed symmetric)
  static SymmetricRankTwoTensorTempl<T> timesTranspose(const RankTwoTensorTempl<T> &);
  static SymmetricRankTwoTensorTempl<T> timesTranspose(const SymmetricRankTwoTensorTempl<T> &);

  /// return the marix plus its transpose A-A^T (guaranteed symmetric)
  static SymmetricRankTwoTensorTempl<T> plusTranspose(const RankTwoTensorTempl<T> &);
  static SymmetricRankTwoTensorTempl<T> plusTranspose(const SymmetricRankTwoTensorTempl<T> &);

  /// Returns the matrix squared
  SymmetricRankTwoTensorTempl<T> sqr() const;

  /// Returns the trace
  T tr() const { return _vals[0] + _vals[1] + _vals[2]; }

  /// Set all components to zero
  void zero();

  /**
   * Gets the value for the index specified.  Takes index = 0,1,2
   * used for const
   */
  inline T operator()(unsigned int i) const { return _vals[i]; }

  /**
   * rotates the tensor data given a rank two tensor rotation tensor
   * _vals[i][j] = R_ij * R_jl * _vals[k][l]
   * @param R rotation matrix as a TypeTensor
   */
  void rotate(const TypeTensor<T> & R);

  /**
   * Returns a matrix that is the transpose of the matrix this
   * was called on. This is a non-operation.
   */
  SymmetricRankTwoTensorTempl<T> transpose() const;

  /// sets _vals to a, and returns _vals
  SymmetricRankTwoTensorTempl<T> & operator=(const SymmetricRankTwoTensorTempl<T> & a);

  /**
   * Assignment-from-scalar operator.  Used only to zero out vectors.
   */
  template <typename Scalar>
  typename boostcopy::enable_if_c<ScalarTraits<Scalar>::value, SymmetricRankTwoTensorTempl &>::type
  operator=(const Scalar & libmesh_dbg_var(p))
  {
    libmesh_assert_equal_to(p, Scalar(0));
    this->zero();
    return *this;
  }

  /// adds a to _vals
  SymmetricRankTwoTensorTempl<T> & operator+=(const SymmetricRankTwoTensorTempl<T> & a);

  /// returns _vals + a
  template <typename T2>
  SymmetricRankTwoTensorTempl<typename CompareTypes<T, T2>::supertype>
  operator+(const SymmetricRankTwoTensorTempl<T2> & a) const;

  /// sets _vals -= a and returns vals
  SymmetricRankTwoTensorTempl<T> & operator-=(const SymmetricRankTwoTensorTempl<T> & a);

  /// returns _vals - a
  template <typename T2>
  SymmetricRankTwoTensorTempl<typename CompareTypes<T, T2>::supertype>
  operator-(const SymmetricRankTwoTensorTempl<T2> & a) const;

  /// returns -_vals
  SymmetricRankTwoTensorTempl<T> operator-() const;

  /// performs _vals *= a
  SymmetricRankTwoTensorTempl<T> & operator*=(const T & a);

  /// returns _vals*a
  template <typename T2>
  auto operator*(const T2 & a) const ->
      typename std::enable_if<ScalarTraits<T2>::value,
                              SymmetricRankTwoTensorTempl<decltype(T() * T2())>>::type;

  /// performs _vals /= a
  SymmetricRankTwoTensorTempl<T> & operator/=(const T & a);

  /// returns _vals/a
  template <typename T2>
  auto operator/(const T2 & a) const ->
      typename std::enable_if<ScalarTraits<T2>::value,
                              SymmetricRankTwoTensorTempl<decltype(T() / T2())>>::type;

  /// Defines multiplication with a vector to get a vector
  template <typename T2>
  TypeVector<typename CompareTypes<T, T2>::supertype> operator*(const TypeVector<T2> & a) const;

  /// Defines logical equality with another SymmetricRankTwoTensorTempl<T2>
  template <typename T2>
  bool operator==(const SymmetricRankTwoTensorTempl<T2> & a) const;

  /// Defines logical inequality with another SymmetricRankTwoTensorTempl<T2>
  template <typename T2>
  bool operator!=(const SymmetricRankTwoTensorTempl<T2> & a) const;

  /// Sets _vals to the values in a ColumnMajorMatrix (must be 3x3)
  SymmetricRankTwoTensorTempl<T> & operator=(const ColumnMajorMatrixTempl<T> & a);

  /// returns _vals_ij * a_ij (sum on i, j)
  T doubleContraction(const SymmetricRankTwoTensorTempl<T> & a) const;

  /// returns C_ijkl = a_ij * b_kl
  SymmetricRankFourTensorTempl<T> outerProduct(const SymmetricRankTwoTensorTempl<T> & a) const;

  /// returns C_ijkl = a_ik * b_jl
  SymmetricRankFourTensorTempl<T> mixedProductIkJl(const SymmetricRankTwoTensorTempl<T> & a) const;

  /// returns C_ijkl = a_jk * b_il
  SymmetricRankFourTensorTempl<T> mixedProductJkIl(const SymmetricRankTwoTensorTempl<T> & a) const;

  /// returns C_ijkl = a_il * b_jk
  SymmetricRankFourTensorTempl<T> mixedProductIlJk(const SymmetricRankTwoTensorTempl<T> & a) const;

  /// returns C_iklm = a_ij * b_jklm
  SymmetricRankFourTensorTempl<T>
  mixedProductIjJklm(const SymmetricRankFourTensorTempl<T> & a) const;

  /// returns C_iklm = a_jm * b_ijkl
  SymmetricRankFourTensorTempl<T>
  mixedProductJmIjkl(const SymmetricRankFourTensorTempl<T> & b) const;

  /// returns C_iklm = a_jk * b_ijlm
  SymmetricRankFourTensorTempl<T>
  mixedProductJkIjlm(const SymmetricRankFourTensorTempl<T> & b) const;

  /// return positive projection tensor of eigen-decomposition
  template <typename T2 = T>
  typename std::enable_if<MooseUtils::IsLikeReal<T2>::value, SymmetricRankFourTensorTempl<T>>::type
  positiveProjectionEigenDecomposition(std::vector<T> &, SymmetricRankTwoTensorTempl<T> &) const;
  template <typename T2 = T>
  typename std::enable_if<!MooseUtils::IsLikeReal<T2>::value, SymmetricRankFourTensorTempl<T>>::type
  positiveProjectionEigenDecomposition(std::vector<T> &, SymmetricRankTwoTensorTempl<T> &) const;

  /// returns A_ij - de_ij*tr(A)/3, where A are the _vals
  SymmetricRankTwoTensorTempl<T> deviatoric() const;

  /// returns the trace of the tensor, ie _vals[i][i] (sum i = 0, 1, 2)
  T trace() const;

  /// retuns the inverse of the tensor
  SymmetricRankTwoTensorTempl<T> inverse() const;

  /**
   * Denote the _vals[i][j] by A_ij, then this returns
   * d(trace)/dA_ij
   */
  SymmetricRankTwoTensorTempl<T> dtrace() const;

  /**
   * Denote the _vals[i][j] by A_ij, then
   * S_ij = A_ij - de_ij*tr(A)/3
   * Then this returns (S_ij + S_ji)*(S_ij + S_ji)/8
   * Note the explicit symmeterisation
   */
  T generalSecondInvariant() const;

  /**
   * Calculates the second invariant (I2) of a tensor
   */
  T secondInvariant() const;

  /**
   * Denote the _vals[i][j] by A_ij, then this returns
   * d(secondInvariant)/dA_ij
   */
  SymmetricRankTwoTensorTempl<T> dsecondInvariant() const;

  /**
   * Denote the _vals[i][j] by A_ij, then this returns
   * d^2(secondInvariant)/dA_ij/dA_kl
   */
  SymmetricRankFourTensorTempl<T> d2secondInvariant() const;

  /**
   * Sin(3*Lode_angle)
   * If secondInvariant() <= r0 then return r0_value
   * This is to gaurd against precision-loss errors.
   * Note that sin(3*Lode_angle) is not defined for secondInvariant() = 0
   */
  template <typename T2 = T>
  typename std::enable_if<MooseUtils::IsLikeReal<T2>::value, T>::type
  sin3Lode(const T & r0, const T & r0_value) const;
  template <typename T2 = T>
  typename std::enable_if<!MooseUtils::IsLikeReal<T2>::value, T>::type
  sin3Lode(const T & r0, const T & r0_value) const;

  /**
   * d(sin3Lode)/dA_ij
   * If secondInvariant() <= r0 then return zero
   * This is to gaurd against precision-loss errors.
   * Note that sin(3*Lode_angle) is not defined for secondInvariant() = 0
   */
  template <typename T2 = T>
  typename std::enable_if<MooseUtils::IsLikeReal<T2>::value, SymmetricRankTwoTensorTempl<T>>::type
  dsin3Lode(const T & r0) const;
  template <typename T2 = T>
  typename std::enable_if<!MooseUtils::IsLikeReal<T2>::value, SymmetricRankTwoTensorTempl<T>>::type
  dsin3Lode(const T & r0) const;

  /**
   * d^2(sin3Lode)/dA_ij/dA_kl
   * If secondInvariant() <= r0 then return zero
   * This is to gaurd against precision-loss errors.
   * Note that sin(3*Lode_angle) is not defined for secondInvariant() = 0
   */
  template <typename T2 = T>
  typename std::enable_if<MooseUtils::IsLikeReal<T2>::value, SymmetricRankFourTensorTempl<T>>::type
  d2sin3Lode(const T & r0) const;
  template <typename T2 = T>
  typename std::enable_if<!MooseUtils::IsLikeReal<T2>::value, SymmetricRankFourTensorTempl<T>>::type
  d2sin3Lode(const T & r0) const;

  /**
   * Denote the _vals[i][j] by A_ij, then
   * S_ij = A_ij - de_ij*tr(A)/3
   * Then this returns det(S + S.transpose())/2
   * Note the explicit symmeterisation
   */
  T thirdInvariant() const;

  /**
   * Denote the _vals[i][j] by A_ij, then
   * this returns d(thirdInvariant()/dA_ij
   */
  SymmetricRankTwoTensorTempl<T> dthirdInvariant() const;

  /**
   * Denote the _vals[i][j] by A_ij, then this returns
   * d^2(thirdInvariant)/dA_ij/dA_kl
   */
  SymmetricRankFourTensorTempl<T> d2thirdInvariant() const;

  /**
   * Denote the _vals[i][j] by A_ij, then this returns
   * d(det)/dA_ij
   */
  SymmetricRankTwoTensorTempl<T> ddet() const;

  /// Print the rank two tensor
  void print(std::ostream & stm = Moose::out) const;

  /// Print the Real part of the DualReal rank two tensor
  void printReal(std::ostream & stm = Moose::out) const;

  /// Print the Real part of the DualReal rank two tensor along with its first nDual dual numbers
  void printDualReal(unsigned int nDual, std::ostream & stm = Moose::out) const;

  /// Add identity times a to _vals
  void addIa(const T & a);

  /// Sqrt(_vals[i][j]*_vals[i][j])
  T L2norm() const;

  /**
   * sets _vals[0][0], _vals[0][1], _vals[1][0], _vals[1][1] to input,
   * and the remainder to zero
   */
  void surfaceFillFromInputVector(const std::vector<T> & input);

  /**
   * computes eigenvalues, assuming tens is symmetric, and places them
   * in ascending order in eigvals
   */
  void symmetricEigenvalues(std::vector<T> & eigvals) const;

  /**
   * computes and returns the permutation matrix P
   * @param old_elements is the original row/column numbering
   * @param new_elements is the permuted row/column numbering
   * Dual numbers are permuted as well
   * P * A permutes rows and A * P^T permutes columns
   */
  SymmetricRankTwoTensorTempl<T>
  permutationTensor(const std::array<unsigned int, LIBMESH_DIM> & old_elements,
                    const std::array<unsigned int, LIBMESH_DIM> & new_elements) const;

  /**
   * computes and returns the Givens rotation matrix R
   * @param row1 is the row number of the first component to rotate
   * @param row2 is the row number of the second component to rotate
   * @param col is the column number of the components to rotate
   * consider a SymmetricRankTwoTensor A = [ a11 a12 a13
   *                                a21 a22 a23
   *                                a31 a32 a33]
   * and we want to rotate a21 and a31. Then row1 = 1, row2 = 2, col = 0.
   * It retunrs the Givens rotation matrix R of this tensor A such that R * A rotates the second
   * component to zero, i.e. R * A = [ a11 a12 a13
   *                                     r a22 a23
   *                                     0 a32 a33]
   * A DualReal instantiation is available to rotate dual numbers as well.
   */
  template <typename T2 = T>
  typename std::enable_if<MooseUtils::IsLikeReal<T2>::value, SymmetricRankTwoTensorTempl<T>>::type
  givensRotation(unsigned int row1, unsigned int row2, unsigned int col) const;
  template <typename T2 = T>
  typename std::enable_if<!MooseUtils::IsLikeReal<T2>::value, SymmetricRankTwoTensorTempl<T>>::type
  givensRotation(unsigned int, unsigned int, unsigned int) const;

  /// computes the Hessenberg form of this matrix A and its unitary transformation U such that A = U * H * U^T
  void hessenberg(SymmetricRankTwoTensorTempl<T> & H, SymmetricRankTwoTensorTempl<T> & U) const;

  /// computes the QR factorization such that A = Q * R, where Q is the unitary matrix and R an upper triangular matrix
  void QR(SymmetricRankTwoTensorTempl<T> & Q,
          SymmetricRankTwoTensorTempl<T> & R,
          unsigned int dim = SymmetricRankTwoTensorTempl<T>::N) const;

  /**
   * computes eigenvalues and eigenvectors, assuming tens is symmetric, and places them
   * in ascending order in eigvals.  eigvecs is a matrix with the first column
   * being the first eigenvector, the second column being the second, etc.
   */
  void symmetricEigenvaluesEigenvectors(std::vector<T> & eigvals,
                                        SymmetricRankTwoTensorTempl<T> & eigvecs) const;

  /**
   * computes eigenvalues, and their symmetric derivatives wrt vals,
   * assuming tens is symmetric
   * @param eigvals are the eigenvalues of the matrix, in ascending order
   * @param deigvals Here digvals[i](j,k) = (1/2)*(d(eigvals[i])/dA_jk + d(eigvals[i]/dA_kj))
   * Note the explicit symmeterisation here.
   * For equal eigenvalues, these derivatives are not gauranteed to
   * be the ones you expect, since the derivatives in this case are
   * often defined by continuation from the un-equal case, and that is
   * too sophisticated for this routine.
   */
  void dsymmetricEigenvalues(std::vector<T> & eigvals,
                             std::vector<SymmetricRankTwoTensorTempl<T>> & deigvals) const;

  /**
   * Computes second derivatives of Eigenvalues of a rank two tensor
   * @param deriv store second derivative of the current tensor in here
   */
  void d2symmetricEigenvalues(std::vector<SymmetricRankFourTensorTempl<T>> & deriv) const;

  /**
   * Uses the petscblaslapack.h LAPACKsyev_ routine to find, for symmetric _vals:
   *  (1) the eigenvalues (if calculation_type == "N")
   *  (2) the eigenvalues and eigenvectors (if calculation_type == "V")
   * @param calculation_type If "N" then calculation eigenvalues only
   * @param eigvals Eigenvalues are placed in this array, in ascending order
   * @param a Eigenvectors are placed in this array if calculation_type == "V".
   * See code in dsymmetricEigenvalues for extracting eigenvectors from the a output.
   */
  void syev(const char * calculation_type, std::vector<T> & eigvals, std::vector<T> & a) const;

  /**
   * Uses the petscblaslapack.h LAPACKsyev_ routine to perform RU decomposition and obtain the
   * rotation tensor.
   */
  void getRUDecompositionRotation(SymmetricRankTwoTensorTempl<T> & rot) const;

  /**
   * This function initializes random seed based on a user-defined number.
   */
  static void initRandom(unsigned int);

  /**
   * This function generates a random symmetric rank two tensor.
   * The first real scales the random number.
   * The second real offsets the uniform random number
   */
  static SymmetricRankTwoTensorTempl<T> genRandomSymmTensor(T, T);

  /// SymmetricRankTwoTensorTempl<T> from outer product of a vector with itself
  void vectorSelfOuterProduct(const TypeVector<T> &);

  /// Return real tensor of a rank two tensor
  void fillRealTensor(TensorValue<T> &);

  ///Assigns value to the columns of a specified row
  void fillRow(unsigned int, const TypeVector<T> &);

  ///Assigns value to the rows of a specified column
  void fillColumn(unsigned int, const TypeVector<T> &);

  /// returns this_ij * b_ijkl
  SymmetricRankTwoTensorTempl<T>
  initialContraction(const SymmetricRankFourTensorTempl<T> & b) const;

  /// set the tensor to the identity matrix
  void setToIdentity();

  static constexpr unsigned int N = 6;

private:
  static constexpr std::array<Real, N> identityCoords = {1, 1, 1, 0, 0, 0};

  // tensor components
  std::array<T, N> _vals;

  /// as std::sqrt is not constexpr we need to define this here ourselves
  static constexpr Real SQRT2 = 1.4142135623730951;

  template <class T2>
  friend void dataStore(std::ostream &, SymmetricRankTwoTensorTempl<T2> &, void *);

  template <class T2>
  friend void dataLoad(std::istream &, SymmetricRankTwoTensorTempl<T2> &, void *);
  template <class T2>
  friend class SymmetricRankFourTensorTempl;
};

namespace MetaPhysicL
{
template <typename T>
struct RawType<SymmetricRankTwoTensorTempl<T>>
{
  typedef SymmetricRankTwoTensorTempl<typename RawType<T>::value_type> value_type;

  static value_type value(const SymmetricRankTwoTensorTempl<T> & in)
  {
    value_type ret;
    for (unsigned int i = 0; i < SymmetricRankTwoTensorTempl<T>::N; ++i)
      ret(i) = raw_value(in(i));

    return ret;
  }
};
}

template <typename T>
template <typename T2>
auto
SymmetricRankTwoTensorTempl<T>::operator*(const T2 & a) const ->
    typename std::enable_if<ScalarTraits<T2>::value,
                            SymmetricRankTwoTensorTempl<decltype(T() * T2())>>::type
{
  SymmetricRankTwoTensorTempl<decltype(T() * T2())> result;
  for (std::size_t i = 0; i < N; ++i)
    result._vals[i] = _vals[i] * a;
  return result;
}

template <typename T>
template <typename T2>
auto
SymmetricRankTwoTensorTempl<T>::operator/(const T2 & a) const ->
    typename std::enable_if<ScalarTraits<T2>::value,
                            SymmetricRankTwoTensorTempl<decltype(T() / T2())>>::type
{
  SymmetricRankTwoTensorTempl<decltype(T() / T2())> result;
  for (std::size_t i = 0; i < N; ++i)
    result._vals[i] = _vals[i] / a;
  return result;
}

/// Defines multiplication with a vector to get a vector
template <typename T>
template <typename T2>
TypeVector<typename CompareTypes<T, T2>::supertype>
SymmetricRankTwoTensorTempl<T>::operator*(const TypeVector<T2> & a) const
{
  TypeVector<typename CompareTypes<T, T2>::supertype> ret;
  ret(0) = a(0) * _vals[0] + a(1) * _vals[5] + a(2) * _vals[4];
  ret(1) = a(0) * _vals[5] + a(1) * _vals[1] + a(2) * _vals[3];
  ret(2) = a(0) * _vals[4] + a(1) * _vals[3] + a(2) * _vals[2];
}

template <typename T>
template <typename T2>
typename std::enable_if<MooseUtils::IsLikeReal<T2>::value, SymmetricRankFourTensorTempl<T>>::type
SymmetricRankTwoTensorTempl<T>::positiveProjectionEigenDecomposition(
    std::vector<T> & eigval, SymmetricRankTwoTensorTempl<T> & eigvec) const
{
  // The calculate of projection tensor follows
  // C. Miehe and M. Lambrecht, Commun. Numer. Meth. Engng 2001; 17:337~353

  // Compute eigenvectors and eigenvalues of this tensor
  this->symmetricEigenvaluesEigenvectors(eigval, eigvec);

  // Separate out positive and negative eigen values
  std::array<T, N> epos;
  std::array<T, N> d;
  for (unsigned int i = 0; i < N; ++i)
  {
    epos[i] = (std::abs(eigval[i]) + eigval[i]) / 2.0;
    d[i] = 0 < eigval[i] ? 1.0 : 0.0;
  }

  // projection tensor
  SymmetricRankFourTensorTempl<T> proj_pos;
  SymmetricRankFourTensorTempl<T> Gab, Gba;
  SymmetricRankTwoTensorTempl<T> Ma, Mb;

  for (unsigned int a = 0; a < N; ++a)
  {
    Ma.vectorOuterProduct(eigvec.column(a), eigvec.column(a));
    proj_pos += d[a] * Ma.outerProduct(Ma);
  }

  for (unsigned int a = 0; a < N; ++a)
    for (unsigned int b = 0; b < a; ++b)
    {
      Ma.vectorOuterProduct(eigvec.column(a), eigvec.column(a));
      Mb.vectorOuterProduct(eigvec.column(b), eigvec.column(b));

      Gab = Ma.mixedProductIkJl(Mb) + Ma.mixedProductIlJk(Mb);
      Gba = Mb.mixedProductIkJl(Ma) + Mb.mixedProductIlJk(Ma);

      T theta_ab;
      if (!MooseUtils::absoluteFuzzyEqual(eigval[a], eigval[b]))
        theta_ab = 0.5 * (epos[a] - epos[b]) / (eigval[a] - eigval[b]);
      else
        theta_ab = 0.25 * (d[a] + d[b]);

      proj_pos += theta_ab * (Gab + Gba);
    }
  return proj_pos;
}

template <typename T>
template <typename T2>
typename std::enable_if<!MooseUtils::IsLikeReal<T2>::value, SymmetricRankFourTensorTempl<T>>::type
SymmetricRankTwoTensorTempl<T>::positiveProjectionEigenDecomposition(
    std::vector<T> &, SymmetricRankTwoTensorTempl<T> &) const
{
  mooseError(
      "positiveProjectionEigenDecomposition is only available for ordered tensor component types");
}

template <typename T>
template <typename T2>
typename std::enable_if<MooseUtils::IsLikeReal<T2>::value, SymmetricRankTwoTensorTempl<T>>::type
SymmetricRankTwoTensorTempl<T>::givensRotation(unsigned int row1,
                                               unsigned int row2,
                                               unsigned int col) const
{
  T c, s;
  T a = (*this)(row1, col);
  T b = (*this)(row2, col);

  if (MooseUtils::absoluteFuzzyEqual(b, 0.0) && MooseUtils::absoluteFuzzyEqual(a, 0.0))
  {
    c = a < 0.0 ? -1.0 : 1.0;
    s = 0.0;
  }
  else if (std::abs(a) > std::abs(b))
  {
    T t = b / a;
    Real sgn = a < 0.0 ? -1.0 : 1.0;
    T u = sgn * std::sqrt(1.0 + t * t);
    c = 1.0 / u;
    s = c * t;
  }
  else
  {
    T t = a / b;
    Real sgn = b < 0.0 ? -1.0 : 1.0;
    T u = sgn * std::sqrt(1.0 + t * t);
    s = 1.0 / u;
    c = s * t;
  }

  SymmetricRankTwoTensorTempl<T> R(initIdentity);
  R(row1, row1) = c;
  R(row1, row2) = s;
  R(row2, row1) = -s;
  R(row2, row2) = c;

  return R;
}

template <typename T>
template <typename T2>
typename std::enable_if<!MooseUtils::IsLikeReal<T2>::value, SymmetricRankTwoTensorTempl<T>>::type
SymmetricRankTwoTensorTempl<T>::givensRotation(unsigned int, unsigned int, unsigned int) const
{
  mooseError("givensRotation is only available for ordered tensor component types");
}

template <typename T>
template <typename T2>
typename std::enable_if<MooseUtils::IsLikeReal<T2>::value, T>::type
SymmetricRankTwoTensorTempl<T>::sin3Lode(const T & r0, const T & r0_value) const
{
  T bar = secondInvariant();
  if (bar <= r0)
    // in this case the Lode angle is not defined
    return r0_value;
  else
    // the min and max here gaurd against precision-loss when bar is tiny but nonzero.
    return std::max(std::min(-1.5 * std::sqrt(3.0) * thirdInvariant() / std::pow(bar, 1.5), 1.0),
                    -1.0);
}

template <typename T>
template <typename T2>
typename std::enable_if<!MooseUtils::IsLikeReal<T2>::value, T>::type
SymmetricRankTwoTensorTempl<T>::sin3Lode(const T &, const T &) const
{
  mooseError("sin3Lode is only available for ordered tensor component types");
}

template <typename T>
template <typename T2>
typename std::enable_if<MooseUtils::IsLikeReal<T2>::value, SymmetricRankTwoTensorTempl<T>>::type
SymmetricRankTwoTensorTempl<T>::dsin3Lode(const T & r0) const
{
  T bar = secondInvariant();
  if (bar <= r0)
    return SymmetricRankTwoTensorTempl<T>();
  else
    return -1.5 * std::sqrt(3.0) *
           (dthirdInvariant() / std::pow(bar, 1.5) -
            1.5 * dsecondInvariant() * thirdInvariant() / std::pow(bar, 2.5));
}

template <typename T>
template <typename T2>
typename std::enable_if<!MooseUtils::IsLikeReal<T2>::value, SymmetricRankTwoTensorTempl<T>>::type
SymmetricRankTwoTensorTempl<T>::dsin3Lode(const T &) const
{
  mooseError("dsin3Lode is only available for ordered tensor component types");
}

template <typename T>
template <typename T2>
typename std::enable_if<MooseUtils::IsLikeReal<T2>::value, SymmetricRankFourTensorTempl<T>>::type
SymmetricRankTwoTensorTempl<T>::d2sin3Lode(const T & r0) const
{
  T bar = secondInvariant();
  if (bar <= r0)
    return SymmetricRankFourTensorTempl<T>();

  T J3 = thirdInvariant();
  SymmetricRankTwoTensorTempl<T> dII = dsecondInvariant();
  SymmetricRankTwoTensorTempl<T> dIII = dthirdInvariant();
  SymmetricRankFourTensorTempl<T> deriv =
      d2thirdInvariant() / std::pow(bar, 1.5) - 1.5 * d2secondInvariant() * J3 / std::pow(bar, 2.5);

  for (unsigned i = 0; i < N; ++i)
    for (unsigned j = 0; j < N; ++j)
      for (unsigned k = 0; k < N; ++k)
        for (unsigned l = 0; l < N; ++l)
          deriv(i, j, k, l) +=
              (-1.5 * dII(i, j) * dIII(k, l) - 1.5 * dIII(i, j) * dII(k, l)) / std::pow(bar, 2.5) +
              1.5 * 2.5 * dII(i, j) * dII(k, l) * J3 / std::pow(bar, 3.5);

  deriv *= -1.5 * std::sqrt(3.0);
  return deriv;
}

template <typename T>
template <typename T2>
typename std::enable_if<!MooseUtils::IsLikeReal<T2>::value, SymmetricRankFourTensorTempl<T>>::type
SymmetricRankTwoTensorTempl<T>::d2sin3Lode(const T &) const
{
  mooseError("d2sin3Lode is only available for ordered tensor component types");
}
