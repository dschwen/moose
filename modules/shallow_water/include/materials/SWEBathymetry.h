//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

class Function;

/**
 * Provides bathymetry property b(x,y) as a MaterialProperty for face use.
 */
class SWEBathymetry : public Material
{
public:
  static InputParameters validParams();

  SWEBathymetry(const InputParameters & parameters);
  virtual ~SWEBathymetry();

protected:
  virtual void computeQpProperties() override;

  const Function & _bed;
  MaterialProperty<Real> & _b;
};

