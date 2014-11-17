#include "RankFourTensor.h"
#include "RankTwoTensor.h"

// Any other includes here
#include <vector>
#include "libmesh/tensor_value.h"
#include "MaterialProperty.h"
#include "libmesh/libmesh.h"
#include <ostream>

extern "C" void FORTRAN_CALL(dsyev) ( ... ); // eigenvalue and eigenvectors for symmetric matrix from LAPACK
extern "C" void FORTRAN_CALL(dgeev) ( ... ); // eigenvalue and eigenvectors for general matrix from LAPACK
extern "C" void FORTRAN_CALL(dgetri) ( ... ); // matrix inversion routine from LAPACK
extern "C" void FORTRAN_CALL(dgetrf) ( ... ); // matrix inversion routine from LAPACK


MooseEnum
RankFourTensor::fillMethodEnum()
{
  return MooseEnum("antisymmetric symmetric9 symmetric21 general_isotropic symmetric_isotropic antisymmetric_isotropic general");
}

RankFourTensor::RankFourTensor() :
    _symmetry(isotropic),
    _data(2)
{
}

RankFourTensor::RankFourTensor(Symmetry s) :
    _symmetry(s),
    _data(int(s))
{
}

Real &
RankFourTensor::operator()(unsigned int i, unsigned int j, unsigned int k, unsigned int l)
{
  collapseSymmetryToNone();
  return _data[((i * N + j) * N + k) * N + l];
}

Real
RankFourTensor::operator()(unsigned int i, unsigned int j, unsigned int k, unsigned int l) const
{
  switch (_symmetry)
  {
    case none :
      return _data[((i * N + j) * N + k) * N + l];

    case isotropic :
    {
      // https://en.wikipedia.org/wiki/Linear_elasticity#Isotropic_homogeneous_media
      const Real K = _data[0],
            mu = _data[1];
      return K*(i==j)*(k==l) + mu*((i==k)*(j==l) + (i==l)*(j==k) - 2.0/3.0*(i==j)*(k==l));
    }

    default:
      mooseError("Invalid tensor symmetry.");
  }
}

void
RankFourTensor::zero()
{
  _symmetry = isotropic;
  _data.resize(2);
  _data[0] = 0.0;
  _data[1] = 0.0;
}

RankFourTensor &
RankFourTensor::operator=(const RankFourTensor & b)
{
  _data = b._data;
  _symmetry = b._symmetry;
  return *this;
}

RankTwoTensor
RankFourTensor::operator*(const RankTwoTensor & b) const
{
  return RankTwoMultiply(b);
}

RealTensorValue
RankFourTensor::operator*(const RealTensorValue & b) const
{
  return RankTwoMultiply(b);
}

template <typename T>
T
RankFourTensor::RankTwoMultiply(const T & b) const
{
  const RankFourTensor & a = *this;
  T result;

  switch (_symmetry)
  {
    case none:
    case isotropic:
      for (unsigned int i = 0; i < N; ++i)
        for (unsigned int j = 0; j < N; ++j)
          for (unsigned int k = 0; k < N; ++k)
            for (unsigned int l = 0; l < N; ++l)
              result(i,j) += a(i,j,k,l) * b(k,l);
      break;

    default:
      mooseError("Invalid tensor symmetry.");
  }

  return result;
}

RankFourTensor
RankFourTensor::operator*(const Real & a) const
{
  RankFourTensor result(_symmetry);
  // we assume that the elements stored for all symmetries can be multiplied individually
  for (unsigned int i = 0; i < _data.size(); ++i)
    result._data[i] = _data[i]*a;
  return result;
}

RankFourTensor &
RankFourTensor::operator*=(const Real & a)
{
  // we assume that the elements stored for all symmetries can be multiplied individually
  for (unsigned int i = 0; i < _data.size(); ++i)
    _data[i] *= a;
  return *this;
}

RankFourTensor
RankFourTensor::operator/(const Real & a) const
{
  RankFourTensor result(_symmetry);
  // we assume that the elements stored for all symmetries can be divided individually
  for (unsigned int i = 0; i < _data.size(); ++i)
    result._data[i] = _data[i]/a;
  return result;
}

RankFourTensor &
RankFourTensor::operator/=(const Real & a)
{
  // we assume that the elements stored for all symmetries can be divided individually
  for (unsigned int i = 0; i < _data.size(); ++i)
    _data[i] /= a;
  return *this;
}

RankFourTensor
RankFourTensor::operator+(const RankFourTensor & b) const
{
  const RankFourTensor & a = *this;

  if (a._symmetry == b._symmetry)
  {
    // for identical symmetries just add components
    RankFourTensor result(_symmetry);

    for (unsigned int i = 0; i < _data.size(); ++i)
      result._data[i] += a._data[i] + b._data[i];
    return result;
  }
  else
  {
    // for all other cases return no symmetry result
    RankFourTensor result(none);

    for (unsigned int i = 0; i < N; ++i)
      for (unsigned int j = 0; j < N; ++j)
        for (unsigned int k = 0; k < N; ++k)
          for (unsigned int l = 0; l < N; ++l)
            result(i,j,k,l) = a(i,j,k,l) + b(i,j,k,l);
    return result;
  }
}

RankFourTensor &
RankFourTensor::operator+=(const RankFourTensor & b)
{
  RankFourTensor & a = *this;

  if (a._symmetry == b._symmetry)
  {
    // for identical symmetries just add components
    for (unsigned int i = 0; i < _data.size(); ++i)
      _data[i] += b._data[i];
  }
  else
  {
    // for all other cases downgrade symmetry to none
    collapseSymmetryToNone();

    for (unsigned int i = 0; i < N; ++i)
      for (unsigned int j = 0; j < N; ++j)
        for (unsigned int k = 0; k < N; ++k)
          for (unsigned int l = 0; l < N; ++l)
            a(i,j,k,l) += a(i,j,k,l);
  }

  return a;
}

RankFourTensor
RankFourTensor::operator-(const RankFourTensor & b) const
{
  const RankFourTensor & a = *this;

  if (a._symmetry == b._symmetry)
  {
    // for identical symmetries just add components
    RankFourTensor result(_symmetry);

    for (unsigned int i = 0; i < _data.size(); ++i)
      result._data[i] += a._data[i] - b._data[i];
    return result;
  }
  else
  {
    // for all other cases return no symmetry result
    RankFourTensor result(none);

    for (unsigned int i = 0; i < N; ++i)
      for (unsigned int j = 0; j < N; ++j)
        for (unsigned int k = 0; k < N; ++k)
          for (unsigned int l = 0; l < N; ++l)
            result(i,j,k,l) = a(i,j,k,l) - b(i,j,k,l);
    return result;
  }
}

RankFourTensor &
RankFourTensor::operator-=(const RankFourTensor & b)
{
  RankFourTensor & a = *this;

  if (a._symmetry == b._symmetry)
  {
    // for identical symmetries just add components
    for (unsigned int i = 0; i < _data.size(); ++i)
      _data[i] -= b._data[i];
  }
  else
  {
    // for all other cases downgrade symmetry to none
    collapseSymmetryToNone();

    for (unsigned int i = 0; i < N; ++i)
      for (unsigned int j = 0; j < N; ++j)
        for (unsigned int k = 0; k < N; ++k)
          for (unsigned int l = 0; l < N; ++l)
            a(i,j,k,l) -= a(i,j,k,l);
  }

  return a;
}

RankFourTensor
RankFourTensor::operator-() const
{
  RankFourTensor result(_symmetry);
  // we assume that the elements stored for all symmetries can be negated individually
  for (unsigned int i = 0; i < _data.size(); ++i)
    result._data[i] = -_data[i];

  return result;
}

RankFourTensor
RankFourTensor::operator*(const RankFourTensor & b) const
{
  const RankFourTensor & a = *this;
  RankFourTensor result(none);

  // TODO: respect symmetries

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          for (unsigned int p = 0; p < N; ++p)
            for (unsigned int q = 0; q < N; ++q)
              result(i,j,k,l) += a(i,j,p,q) * b(p,q,k,l);

  return result;
}

Real
RankFourTensor::L2norm() const
{
  const RankFourTensor & a = *this;
  Real l2 = 0;

  switch (_symmetry)
  {
    case none:
      for (unsigned int i = 0; i < _data.size(); ++i)
        l2 += _data[i] * _data[i];
      break;

    case isotropic :
      // TODO: simplify
      for (unsigned int i = 0; i < N; ++i)
        for (unsigned int j = 0; j < N; ++j)
          for (unsigned int k = 0; k < N; ++k)
            for (unsigned int l = 0; l < N; ++l)
              l2 += std::pow(a(i,j,k,l), 2.0);
      break;

    default:
      mooseError("Invalid tensor symmetry.");
  }

  return std::sqrt(l2);
}

void
RankFourTensor::collapseSymmetryToNone()
{
  if (_symmetry == none) return;

  const RankFourTensor & a = *this;
  RankFourTensor result(none);

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          result(i,j,k,l) = a(i,j,k,l);

  _data = result._data;
  _symmetry = none;
}

RankFourTensor
RankFourTensor::constructOuterProduct(const RankTwoTensor & b, const RankTwoTensor & c)
{
  RankFourTensor result(none);

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        for (unsigned int l = 0; l < LIBMESH_DIM; ++l)
          result(i,j,k,l) = b(i,j) * c(k,l);

  return result;
}

RankFourTensor
RankFourTensor::outerProduct(const RankTwoTensor & b, const RankTwoTensor & c)
{
  const RankFourTensor & a = *this;

  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      for (unsigned int k = 0; k < LIBMESH_DIM; ++k)
        for (unsigned int l = 0; l < LIBMESH_DIM; ++l)
          a(i,j,k,l) = b(i,j) * c(k,l);

  return result;
}


RankFourTensor
RankFourTensor::invSymm() const
{
  int error;

  const RankFourTensor & a = *this;
  RankFourTensor result;

  unsigned int ntens = N * (N+1) / 2;
  int nskip = N-1;

  // We use the LAPACK matrix inversion routine here.  Form the matrix
  //
  // mat[0]  mat[1]  mat[2]  mat[3]  mat[4]  mat[5]
  // mat[6]  mat[7]  mat[8]  mat[9]  mat[10] mat[11]
  // mat[12] mat[13] mat[14] mat[15] mat[16] mat[17]
  // mat[18] mat[19] mat[20] mat[21] mat[22] mat[23]
  // mat[24] mat[25] mat[26] mat[27] mat[28] mat[29]
  // mat[30] mat[31] mat[32] mat[33] mat[34] mat[35]
  //
  // This is filled from the indpendent components of C assuming
  // the symmetry C_ijkl = C_ijlk = C_jikl.
  //
  // If there are two rank-four tensors X and Y then the reason for
  // this filling becomes apparent if we want to calculate
  // X_ijkl*Y_klmn = Z_ijmn
  // For denote the "mat" versions of X, Y and Z by x, y and z.
  // Then
  // z_ab = x_ac*y_cb
  // Eg
  // z_00 = Z_0000 = X_0000*Y_0000 + X_0011*Y_1111 + X_0022*Y_2200 + 2*X_0001*Y_0100 + 2*X_0002*Y_0200 + 2*X_0012*Y_1200   (the factors of 2 come from the assumed symmetries)
  // z_03 = 2*Z_0001 = X_0000*2*Y_0001 + X_0011*2*Y_1101 + X_0022*2*Y_2201 + 2*X_0001*2*Y_0101 + 2*X_0002*2*Y_0201 + 2*X_0012*2*Y_1201
  // z_22 = 2*Z_0102 = X_0100*2*Y_0002 + X_0111*2*X_1102 + X_0122*2*Y_2202 + 2*X_0101*2*Y_0102 + 2*X_0102*2*Y_0202 + 2*X_0112*2*Y_1202
  // Finally, we use LAPACK to find x^-1, and put it back into rank-4 tensor form
  //
  // mat[0] = C(0,0,0,0)
  // mat[1] = C(0,0,1,1)
  // mat[2] = C(0,0,2,2)
  // mat[3] = C(0,0,0,1)*2
  // mat[4] = C(0,0,0,2)*2
  // mat[5] = C(0,0,1,2)*2

  // mat[6] = C(1,1,0,0)
  // mat[7] = C(1,1,1,1)
  // mat[8] = C(1,1,2,2)
  // mat[9] = C(1,1,0,1)*2
  // mat[10] = C(1,1,0,2)*2
  // mat[11] = C(1,1,1,2)*2

  // mat[12] = C(2,2,0,0)
  // mat[13] = C(2,2,1,1)
  // mat[14] = C(2,2,2,2)
  // mat[15] = C(2,2,0,1)*2
  // mat[16] = C(2,2,0,2)*2
  // mat[17] = C(2,2,1,2)*2

  // mat[18] = C(0,1,0,0)
  // mat[19] = C(0,1,1,1)
  // mat[20] = C(0,1,2,2)
  // mat[21] = C(0,1,0,1)*2
  // mat[22] = C(0,1,0,2)*2
  // mat[23] = C(0,1,1,2)*2

  // mat[24] = C(0,2,0,0)
  // mat[25] = C(0,2,1,1)
  // mat[26] = C(0,2,2,2)
  // mat[27] = C(0,2,0,1)*2
  // mat[28] = C(0,2,0,2)*2
  // mat[29] = C(0,2,1,2)*2

  // mat[30] = C(1,2,0,0)
  // mat[31] = C(1,2,1,1)
  // mat[32] = C(1,2,2,2)
  // mat[33] = C(1,2,0,1)*2
  // mat[34] = C(1,2,0,2)*2
  // mat[35] = C(1,2,1,2)*2

  std::vector<double> mat(ntens * ntens);

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
        {
          if (i==j)
          {
            if (k==l)
              mat[i*ntens+k] = a(i,j,k,l);
            else
              mat[i*ntens+nskip+k+l] += a(i,j,k,l);
          }
          else // i!=j
          {
            if (k==l)
              mat[(nskip+i+j)*ntens+k] = a(i,j,k,l);
            else
              mat[(nskip+i+j)*ntens+nskip+k+l] += a(i,j,k,l); // note the +=, which results in double-counting and is rectified below
          }
        }

  for (unsigned int i = 3; i < ntens; i++)
    for (unsigned int j = 3; j < ntens; j++)
      mat[i*ntens+j] /= 2.0; // because of double-counting above

  // use LAPACK to find the inverse
  error = MatrixInversion(mat, ntens);
  if (error != 0)
    mooseError("Error in Matrix  Inversion in RankFourTensor");

  // build the resulting rank-four tensor
  // using the inverse of the above algorithm
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
        {
          if (i==j)
          {
            if (k==l)
              result(i,j,k,l)=mat[i*ntens+k];
            else
              result(i,j,k,l)=mat[i*ntens+nskip+k+l]/2.0;
          }
          else // i!=j
          {
            if (k==l)
              result(i,j,k,l)=mat[(nskip+i+j)*ntens+k];
            else
              result(i,j,k,l)=mat[(nskip+i+j)*ntens+nskip+k+l]/2.0;
          }
        }

  return result;
}

void
RankFourTensor::rotate(const RealTensorValue & R)
{
  Real temp;

  // isotropic tensors are invariant to rotation
  if (_symmetry == isotropic) return;

  // for all others we downgrade to no symmetry at all
  collapseSymmetryToNone();
  RankFourTensor & a = *this;
  RankFourTensor old = *this;

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
        {
          temp = 0.0;
          for (unsigned int m = 0; m < N; ++m)
            for (unsigned int n = 0; n < N; ++n)
              for (unsigned int o = 0; o < N; ++o)
                for (unsigned int p = 0; p < N; ++p)
                  temp += R(i,m) * R(j,n) * R(k,o) * R(l,p) * old(m,n,o,p);

          a(i,j,k,l) = temp;
        }
}

void
RankFourTensor::print() const
{
  const RankFourTensor & a = *this;

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
    {
      Moose::out << "i = " << i << " j = " << j << std::endl;
      for (unsigned int k = 0; k < N; ++k)
      {
        for (unsigned int l = 0; l < N; ++l)
          Moose::out << std::setw(15) << a(i,j,k,l) << " ";

        Moose::out << std::endl;
      }
    }
}

RankFourTensor
RankFourTensor::transposeMajor() const
{
  const RankFourTensor & a = *this;

  switch (_symmetry)
  {
    case none:
    {
      RankFourTensor result(none);

      for (unsigned int i = 0; i < N; ++i)
        for (unsigned int j = 0; j < N; ++j)
          for (unsigned int k = 0; k < N; ++k)
            for (unsigned int l = 0; l < N; ++l)
              result(i,j,k,l) = a(k,l,i,j);

      return result;
    }

    case isotropic:
      return a;

    default:
      mooseError("Invalid tensor symmetry.");
  }
}

void
RankFourTensor::surfaceFillFromInputVector(const std::vector<Real> & input)
{
  zero();
  RankFourTensor & a = *this;

  if (input.size() == 9)
  {
    //then fill from vector C_1111, C_1112, C_1122, C_1212, C_1222, C_1211, C_2211, C_2212, C_2222
    a(0,0,0,0) = input[0];
    a(0,0,0,1) = input[1];
    a(0,0,1,1) = input[2];
    a(0,1,0,1) = input[3];
    a(0,1,1,1) = input[4];
    a(0,1,0,0) = input[5];
    a(1,1,0,0) = input[6];
    a(1,1,0,1) = input[7];
    a(1,1,1,1) = input[8];

    // fill in remainders from C_ijkl = C_ijlk = C_jikl
    a(0,0,1,0) = a(0,0,0,1);
    a(0,1,1,0) = a(0,1,0,1);
    a(1,0,0,0) = a(0,1,0,0);
    a(1,0,0,1) = a(0,1,0,1);
    a(1,0,1,1) = a(0,1,1,1);
    a(1,0,0,0) = a(0,1,0,0);
    a(1,1,1,0) = a(1,1,0,1);
  }
  else if (input.size() == 2)
  {
    // only two independent constants, C_1111 and C_1122
    a(0,0,0,0) = input[0];
    a(0,0,1,1) = input[1];
    //use symmetries
    a(1,1,1,1) = a(0,0,0,0);
    a(1,1,0,0) = a(0,0,1,1);
    a(0,1,0,1) = 0.5 * (a(0,0,0,0) - a(0,0,1,1));
    a(1,0,0,1) = a(0,1,0,1);
    a(0,1,1,0) = a(0,1,0,1);
    a(1,0,1,0) = a(0,1,0,1);
  }
  else
    mooseError("Please provide correct number of inputs for surface RankFourTensor initialization.");
}

void
RankFourTensor::fillFromInputVector(const std::vector<Real> & input, FillMethod fill_method)
{
  zero();

  switch (fill_method)
  {
    case antisymmetric:
      fillAntisymmetricFromInputVector(input);
      break;
    case symmetric9:
      fillSymmetricFromInputVector(input, false);
      break;
    case symmetric21:
      fillSymmetricFromInputVector(input, true);
      break;
    case general_isotropic:
      fillGeneralIsotropicFromInputVector(input);
      break;
    case symmetric_isotropic:
      fillSymmetricIsotropicFromInputVector(input);
      break;
    case antisymmetric_isotropic:
      fillAntisymmetricIsotropicFromInputVector(input);
      break;
    case general:
      fillGeneralFromInputVector(input);
      break;
    default:
      mooseError("fillFromInputVector called with unknown fill_method of " << fill_method);
  }
}

int
RankFourTensor::MatrixInversion(std::vector<double> & A, int n) const
{
  int return_value, buffer_size = n * 64;

  std::vector<int> ipiv(n);
  std::vector<int> buffer(buffer_size);

  // Following does a LU decomposition of "square matrix A"
  // upon return "A = P*L*U" if return_value == 0
  // Here i use quotes because A is actually an array of length n^2, not a matrix of size n-by-n
  FORTRAN_CALL(dgetrf)(&n, &n, &A[0], &n, &ipiv[0], &return_value);

  // couldn't LU decompose because: illegal value in A; or, A singular
  if (return_value != 0)
    return return_value;

  // get the inverse of A
  FORTRAN_CALL(dgetri)(&n, &A[0], &n, &ipiv[0], &buffer[0], &buffer_size, &return_value);

  return return_value;
}

void
RankFourTensor::fillSymmetricFromInputVector(const std::vector<Real> & input, bool all)
{
  if ((all == true && input.size() != 21) || (all == false && input.size() != 9 ))
    mooseError("Please check the number of entries in the stiffness input vector.");

  zero();
  RankFourTensor & a = *this;

  if (all == true)
  {
    a(0,0,0,0) = input[0]; //C1111
    a(0,0,1,1) = input[1]; //C1122
    a(0,0,2,2) = input[2]; //C1133
    a(0,0,1,2) = input[3]; //C1123
    a(0,0,0,2) = input[4]; //C1113
    a(0,0,0,1) = input[5]; //C1112

    a(1,1,1,1) = input[6]; //C2222
    a(1,1,2,2) = input[7]; //C2233
    a(1,1,1,2) = input[8]; //C2223
    a(0,2,1,1) = input[9]; //C2213  //flipped for filling purposes
    a(0,1,1,1) = input[10]; //C2212 //flipped for filling purposes

    a(2,2,2,2) = input[11]; //C3333
    a(1,2,2,2) = input[12]; //C3323 //flipped for filling purposes
    a(0,2,2,2) = input[13]; //C3313 //flipped for filling purposes
    a(0,1,2,2) = input[14]; //C3312 //flipped for filling purposes

    a(1,2,1,2) = input[15]; //C2323
    a(0,2,1,2) = input[16]; //C2313 //flipped for filling purposes
    a(0,1,1,2) = input[17]; //C2312 //flipped for filling purposes

    a(0,2,0,2) = input[18]; //C1313
    a(0,1,0,2) = input[19]; //C1312 //flipped for filling purposesc

    a(0,1,0,1) = input[20]; //C1212
  }
  else
  {
    a(0,0,0,0) = input[0];   //C1111
    a(0,0,1,1) = input[1];   //C1122
    a(0,0,2,2) = input[2];   //C1133
    a(1,1,1,1) = input[3];   //C2222
    a(1,1,2,2) = input[4];   //C2233
    a(2,2,2,2) = input[5];  //C3333
    a(1,2,1,2) = input[6];  //C2323
    a(0,2,0,2) = input[7];  //C1313
    a(0,1,0,1) = input[8];  //C1212
  }

  //fill in from symmetry relations
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
          a(i,j,l,k) =
          a(j,i,k,l) =
          a(j,i,l,k) =
          a(k,l,i,j) =
          a(l,k,j,i) =
          a(k,l,j,i) =
          a(l,k,i,j) = a(i,j,k,l);
}

void
RankFourTensor::fillAntisymmetricFromInputVector(const std::vector<Real> & input)
{
  if (input.size() != 6)
    mooseError("To use fillAntisymmetricFromInputVector, your input must have size 6.  Yours has size " << input.size());

  zero();
  RankFourTensor & a = *this;

  a(0,1,0,1) = input[0];   //B1212
  a(0,1,0,2) = input[1];   //B1213
  a(0,1,1,2) = input[2];   //B1223

  a(0,2,0,2) = input[3];   //B1313
  a(0,2,1,2) = input[4];   //B1323

  a(1,2,1,2) = input[5];   //B2323

  // symmetry on the two pairs
  a(0,2,0,1) = a(0,1,0,2);
  a(1,2,0,1) = a(0,1,1,2);
  a(1,2,0,2) = a(0,2,1,2);
  // have now got the upper parts of vals[0][1], vals[0][2] and vals[1][2]

  // fill in from antisymmetry relations
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
    {
      a(0,1,j,i) = -a(0,1,i,j);
      a(0,2,j,i) = -a(0,2,i,j);
      a(1,2,j,i) = -a(1,2,i,j);
    }
  // have now got all of vals[0][1], vals[0][2] and vals[1][2]

  // fill in from antisymmetry relations
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
    {
      a(1,0,i,j) = -a(0,1,i,j);
      a(2,0,i,j) = -a(0,2,i,j);
      a(2,1,i,j) = -a(1,2,i,j);
    }
}

void
RankFourTensor::fillGeneralIsotropicFromInputVector(const std::vector<Real> & input)
{
  if (input.size() != 3)
    mooseError("To use fillGeneralIsotropicFromInputVector, your input must have size 3.  Yours has size " << input.size());

  RankFourTensor & a = *this;

  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
        {
          a(i,j,k,l) = input[0] * (i==j) * (k==l) + input[1] * (i==k) * (j==l) + input[1] * (i==l) * (j==k);
          for (unsigned int m = 0 ; m < N ; ++m)
            a(i,j,k,l) += input[2] * PermutationTensor::eps(i, j, m) * PermutationTensor::eps(k, l, m);
        }
}

void
RankFourTensor::fillAntisymmetricIsotropicFromInputVector(const std::vector<Real> & input)
{
  if (input.size() != 1)
    mooseError("To use fillAntisymmetricIsotropicFromInputVector, your input must have size 1. Yours has size " << input.size());

  std::vector<Real> input3(3);
  input3[2] = input[0];
  fillGeneralIsotropicFromInputVector(input3);
}

void
RankFourTensor::fillSymmetricIsotropicFromInputVector(const std::vector<Real> & input)
{
  if (input.size() != 2)
    mooseError("To use fillSymmetricIsotropicFromInputVector, your input must have size 2. Yours has size " << input.size());

  std::vector<Real> input3 = input;
  input3.push_back(0);
  fillGeneralIsotropicFromInputVector(input3);
}

void
RankFourTensor::fillGeneralFromInputVector(const std::vector<Real> & input)
{
  if (input.size() != 81)
    mooseError("To use fillGeneralFromInputVector, your input must have size 81. Yours has size " << input.size());

  _data = input;
  _symmetry = none;
}
