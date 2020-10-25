//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LambdaDiffusion.h"

registerMooseObject("MooseApp", LambdaDiffusion);

defineLegacyParams(LambdaDiffusion);

InputParameters
LambdaDiffusion::validParams()
{
  InputParameters params = LambdaKernel::validParams();
  params.addClassDescription("The Laplacian operator ($-\\nabla \\cdot \\nabla u$), with the weak "
                             "form of $(\\nabla \\phi_i, \\nabla u_h)$.");
  return params;
}

LambdaDiffusion::LambdaDiffusion(const InputParameters & parameters) : LambdaKernel(parameters) {}

void
LambdaDiffusion::computeResidual()
{
  applyResidual([&]() { return _grad_u[_qp] * _grad_test[_i][_qp]; });
}

void
LambdaDiffusion::computeJacobian()
{
  applyJacobian([&]() { return _grad_phi[_j][_qp] * _grad_test[_i][_qp]; });
}
