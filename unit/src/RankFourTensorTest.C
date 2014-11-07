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
RankFourTensorTest::L2normTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, _m0.L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(1.732051, _m1.L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(3.741657, _m2.L2norm(), 0.0001);
}

void
RankFourTensorTest::transposeTest()
{
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (_m0.transposeMajor() - _m0).L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (_m1.transposeMajor() - _m1).L2norm(), 0.0001);
  CPPUNIT_ASSERT_DOUBLES_EQUAL(0, (_m2.transposeMajor() - _m2).L2norm(), 0.0001);
}
