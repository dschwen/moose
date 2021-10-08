//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"

/**
 * A material that couples a material property
 */
class OptionalMaterial : public Material
{
public:
  static InputParameters validParams();

  OptionalMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  const MaterialProperty<Real> * const & _mat_prop1;
  const MaterialProperty<Real> * const & _mat_prop2;
  MaterialProperty<Real> & _out;
};
