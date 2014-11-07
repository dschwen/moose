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
