/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "RankFourTensorTest.h"

CPPUNIT_TEST_SUITE_REGISTRATION( RankFourTensorTest );

RankFourTensorTest::RankFourTensorTest()
{
  _m0 = RankFourTensor();
  _m1 = RankFourTensor();
  _m2 = RankFourTensor();

  const unsigned int N = 3;

  Real c = 0.0;
  for (unsigned int i = 0; i < N; ++i)
    for (unsigned int j = 0; j < N; ++j)
      for (unsigned int k = 0; k < N; ++k)
        for (unsigned int l = 0; l < N; ++l)
        {
          _m0(i,j,k,l) = c;
          _m1(i,j,k,l) = 2.0 * c;
          _m2(i,j,k,l) = 3.0 * c;
          c += 0.178;
        }
}

RankFourTensorTest::~RankFourTensorTest()
{}

void
RankFourTensorTest::SubtractTest()
{
  RankFourTensor i = _m1;
  i -= _m0;
  i -= _m0;

  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, i.L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, (_m2 - _m1 -_m0).L2norm(), 0.0001);
}

void
RankFourTensorTest::AddTest()
{
  RankFourTensor i = _m0;
  i += _m1;
  i += _m2;

  const Real l2 = 74.2240791118355;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(l2 * 6.0, i.L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(l2 * 6.0, (_m2 + _m1 +_m0).L2norm(), 0.0001);
}

void
RankFourTensorTest::DivideTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, (_m2/3.0 - _m0).L2norm(), 0.00001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, (_m0 - _m1/2.0).L2norm(), 0.00001);
}

void
RankFourTensorTest::MultiplyTest()
{
  // scalar
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, (_m0*3 - _m2).L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, (_m1 - _m0*2.0).L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, (_m1*1.5 - _m2).L2norm(), 0.0001);

  // tensor
  Real result_values[] = {
    116.344, 118.625, 120.906, 123.187, 125.469, 127.75, 130.031, 132.312, 134.594, 301.125, 308.539, 315.953,
    323.367, 330.781, 338.195, 345.609, 353.023, 360.437, 485.906, 498.453, 511, 523.546, 536.093, 548.64, 561.187,
    573.734, 586.281, 670.687, 688.367, 706.046, 723.726, 741.406, 759.085, 776.765, 794.445, 812.124, 855.468,
    878.28, 901.093, 923.905, 946.718, 969.53, 992.343, 1015.16, 1037.97, 1040.25, 1068.19, 1096.14, 1124.08,
    1152.03, 1179.98, 1207.92, 1235.87, 1263.81, 1225.03, 1258.11, 1291.19, 1324.26, 1357.34, 1390.42, 1423.5,
    1456.58, 1489.65, 1409.81, 1448.02, 1486.23, 1524.44, 1562.65, 1600.87, 1639.08, 1677.29, 1715.5, 1594.59,
    1637.94, 1681.28, 1724.62, 1767.97, 1811.31, 1854.65, 1898, 1941.34
  };
  RankFourTensor result;
  std::vector<Real> result_vector(result_values, result_values + 81);
  result.fillFromInputVector(result_vector, RankFourTensor::general);

  //(_m0*_m1).printFlat();
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, (_m0*_m1 - _m1*_m0).L2norm(), 0.02);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, (_m0*_m1 - result).L2norm(), 0.02);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, ( (_m0*(_m1+_m2))  - (_m0*_m1+_m0*_m2)).L2norm(), 0.02);

  RankFourTensor i = _m0;
  i *= 2.0;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, (i - _m1).L2norm(), 0.000001);
}

void
RankFourTensorTest::L2normTest()
{
  const Real l2 = 74.2240791118355;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(l2, _m0.L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0 * l2, _m1.L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0 * l2, _m2.L2norm(), 0.0001);
}

void
RankFourTensorTest::transposeTest()
{
  const Real l2 = 46.7974153132415;
  CPPUNIT_ASSERT_DOUBLES_EQUAL(l2, (_m0.transposeMajor() - _m0).L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0 * l2, (_m1.transposeMajor() - _m1).L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(3.0 * l2, (_m2.transposeMajor() - _m2).L2norm(), 0.0001);
}
