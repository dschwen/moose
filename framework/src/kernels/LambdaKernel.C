//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LambdaKernel.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableFE.h"
#include "MooseVariableScalar.h"
#include "SubProblem.h"
#include "NonlinearSystem.h"

#include "libmesh/threads.h"
#include "libmesh/quadrature.h"

defineLegacyParams(LambdaKernel);

InputParameters
LambdaKernel::validParams()
{
  return Kernel::validParams();
}

LambdaKernel::LambdaKernel(const InputParameters & parameters) : Kernel(parameters) {}

void
LambdaKernel::computeResidual()
{
  mooseError("override computeResidual() and call applyResidual(lambda) in there");
}

void
LambdaKernel::computeJacobian()
{
  mooseError("override computeJacobian() and call applyJacobian(lambda) in there");
}

void
LambdaKernel::computeOffDiagJacobian(MooseVariableFEBase &)
{
  mooseError(
      "override computeOffDiagJacobian() and call applyOffDiagJacobian(jvar, lambda) in there");
}
