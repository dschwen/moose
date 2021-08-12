//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GmshPlateGenerator.h"

class GmshHolePlateGenerator : public GmshPlateGenerator
{
public:
  static InputParameters validParams();

  GmshHolePlateGenerator(const InputParameters & parameters);

protected:
  void generateGeometry() override;

  std::vector<Real> _x;
  std::vector<Real> _y;
  std::vector<Real> _r;
  std::size_t _n_holes;
};
