//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoarseningIntegralCompensation.h"
#include "CoarseningIntegralTracker.h"

registerMooseObject("PhaseFieldApp", CoarseningIntegralCompensation);

template <>
InputParameters
validParams<CoarseningIntegralCompensation>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Apply a correction obtained by a CoarseningIntegralTracker to "
                             "compensate for variable integrals due to mesh coarsening.");
  params.addRequiredParam<UserObjectName>("tracker", "Coarsening integral tracker user object");
  return params;
}

CoarseningIntegralCompensation::CoarseningIntegralCompensation(const InputParameters & parameters)
  : Kernel(parameters),
    _tracker(getUserObject<CoarseningIntegralTracker>("tracker")),
    _dt(_fe_problem.dt())
{
}

void
CoarseningIntegralCompensation::precalculateResidual()
{
  _element_correction = _tracker.sourceValue(_current_elem);
}

Real
CoarseningIntegralCompensation::computeQpResidual()
{
  return -_element_correction * _test[_i][_qp] / _dt;
}

Real
CoarseningIntegralCompensation::computeQpJacobian()
{
  return 0;
}
