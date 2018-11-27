//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef RANKTHREETENSORTEST_H
#define RANKTHREETENSORTEST_H

#include "gtest_include.h"

// Moose includes
#include "RankTwoTensor.h"

class RankThreeTensorTest : public ::testing::Test
{
protected:
  void SetUp()
  {
    ASSERT_TRUE(LIBMESH_DIM == 3);

    // generate three rank two tensors with unique components
    for (unsigned int i = 0; i < 3; ++i)
      for (unsigned int j = 0; j < 3; ++j)
      {
        _a(i, j) = i + 0.1 * j + 0.00;
        _b(i, j) = i + 0.1 * j + 0.01;
        _c(i, j) = i + 0.1 * j + 0.02;
      }
  }

  RankTwoTensor _a;
  RankTwoTensor _b;
  RankTwoTensor _c;
};

#endif // RANKTHREETENSORTEST_H
