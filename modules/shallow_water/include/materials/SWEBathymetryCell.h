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
 * Provides bathymetry b as a cell-centered (piecewise-constant) material property.
 * The value is evaluated once per element at a representative point and reused
 * for all quadrature points (volume and side) within that element.
 */
class SWEBathymetryCell : public Material
{
public:
  static InputParameters validParams();

  SWEBathymetryCell(const InputParameters & parameters);

protected:
  virtual void computeQpProperties() override;

  /// Bed elevation function b(x,y[,t])
  const Function & _bed_fun;

  /// Cell-constant bathymetry material property
  MaterialProperty<Real> & _b;

  /// Cache to avoid recomputing per element
  dof_id_type _cached_id;
  Real _b_cell;
};

