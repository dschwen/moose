
//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InclinedDirichletBC.h"

registerMooseObject("MooseApp", InclinedDirichletBC);

InputParameters
InclinedDirichletBC::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addRequiredParam<RealVectorValue>("normal", "Surface normal of the boundary");
  params.declareControllable("normal");
  params.addCoupledVar("displacements", "Displacement variables");
  return params;
}

InclinedDirichletBC::InclinedDirichletBC(const InputParameters & parameters)
  : ADIntegratedBC(parameters), _normal(getParam<RealVectorValue>("normal"))
{
  auto ndisp = coupledComponents("displacements");

  for (std::size_t i = 0; i < ndisp; ++i)
  {
    auto disp = getVar("displacements", i);
    if (disp->number() == _var.number())
      _my_normal = _normal(i);
    else
      _disp_norm.emplace_back(&adCoupledValue("displacements", i), _normal(i));
  }
}

ADReal
InclinedDirichletBC::computeQpResidual()
{
  ADReal sum = 0.0;
  for (auto & dn : _disp_norm)
    sum += (*dn.first)[_qp] * dn.second;

  // std::cout << _u[_qp] * _my_normal + sum << '\n';
  auto res = _u[_qp] * _my_normal + sum;
  return res * _test[_i][_qp];
}
