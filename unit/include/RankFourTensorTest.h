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

#ifndef RANKFOURTENSORTEST_H
#define RANKFOURTENSORTEST_H

//CPPUnit includes
#include "cppunit/extensions/HelperMacros.h"

// Moose includes
#include "RankFourTensor.h"

class RankFourTensorTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE( RankFourTensorTest );

  CPPUNIT_TEST( L2normTest );
  CPPUNIT_TEST( SubtractTest );
  CPPUNIT_TEST( AddTest );
  CPPUNIT_TEST( DivideTest );
  CPPUNIT_TEST( MultiplyTest );
  CPPUNIT_TEST( transposeTest );

  CPPUNIT_TEST_SUITE_END();

public:
  RankFourTensorTest();
  ~RankFourTensorTest();

  void L2normTest();
  void SubtractTest();
  void AddTest();
  void DivideTest();
  void MultiplyTest();
  void transposeTest();

 private:
  RankFourTensor _m0;
  RankFourTensor _m1;
  RankFourTensor _m2;
};

#endif  // RANKFOURTENSORTEST_H
