//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADNodalBC.h"

/**
 * Boundary condition of a Dirichlet type
 *
 * Sets the value in the node
 */
class InclinedDirichletBC : public ADNodalBC
{
public:
  static InputParameters validParams();

  InclinedDirichletBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// The normal vector of the boundary
  const RealVectorValue & _normal;

  /// The other displacements and their normal vector component
  std::vector<std::pair<const ADReal *, Real>> _disp_norm;

  /// the normal vector component associated with the current variable
  Real _my_normal;
};
