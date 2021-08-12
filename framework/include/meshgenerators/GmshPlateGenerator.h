//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GmshGeneratorBase.h"

class GmshPlateGenerator : public GmshGeneratorBase
{
public:
  static InputParameters validParams();

  GmshPlateGenerator(const InputParameters & parameters);

protected:
  void generateGeometry() override;

  Real _xmin;
  Real _xmax;
  Real _ymin;
  Real _ymax;
};
