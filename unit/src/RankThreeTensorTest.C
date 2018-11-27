//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RankThreeTensorTest.h"
#include "RankThreeTensor.h"

TEST_F(RankThreeTensorTest, ranktwoconstruction)
{
  // construct rank three tensor from three rank two tensors
  RankThreeTensor d(_a, _b, _c);

  // check components
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      for (unsigned int k = 0; k < 3; ++k)
        EXPECT_NEAR(i + 0.1 * j + k * 0.01, d(i, j, k), 0.0001);
}

TEST_F(RankThreeTensorTest, multiplyvector)
{
  // construct rank three tensor from three rank two tensors
  RankThreeTensor d(_a, _b, _c);

  // multiply three unit vectors onto tensor
  RankTwoTensor a = d * RealVectorValue(1, 0, 0);
  RankTwoTensor b = d * RealVectorValue(0, 1, 0);
  RankTwoTensor c = d * RealVectorValue(0, 0, 1);

  // check component wise
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
    {
      EXPECT_NEAR(a(i, j), _a(i, j), 0.0001);
      EXPECT_NEAR(b(i, j), _b(i, j), 0.0001);
      EXPECT_NEAR(c(i, j), _c(i, j), 0.0001);
    }

  // check using L2 norm
  EXPECT_NEAR((a - _a).L2norm(), 0, 0.000001);
  EXPECT_NEAR((b - _b).L2norm(), 0, 0.000001);
  EXPECT_NEAR((c - _c).L2norm(), 0, 0.000001);
}
