//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LagrangeInclinedNoDisplacement.h"

registerMooseObject("TensorMechanicsApp", LagrangeInclinedNoDisplacement);

InputParameters
LagrangeInclinedNoDisplacement::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addCoupledVar("displacements", "Displacement variables");
  return params;
}

LagrangeInclinedNoDisplacement::LagrangeInclinedNoDisplacement(const InputParameters & parameters)
  : ADKernel(parameters),
    _fe(FEGenericBase<Real>::build(
        _mesh.dimension(), FEType(_mesh.hasSecondOrderElements() ? SECOND : FIRST, LAGRANGE))),
    _normals(_fe->get_normals()),
    _disp(adCoupledValues("displacements"))
{
}

void
LagrangeInclinedNoDisplacement::precalculateResidual()
{
  // get the higher dim element this lower dim element is based off
  auto * ip = _current_elem->interior_parent();

  // query side id corresponding to current lower dim element
  auto s = ip->which_side_am_i(_current_elem);

  // attach proper quadrature rule
  _fe->attach_quadrature_rule(_assembly.writeableQRule());

  // update normals
  _fe->reinit(ip, s);
}

ADReal
LagrangeInclinedNoDisplacement::computeQpResidual()
{
  ADReal sum = 0.0;
  for (std::size_t i = 0; i < _disp.size(); ++i)
    sum += (*_disp[i])[_qp] * _normals[_qp](i);
  return _u[_qp] - sum;
}
