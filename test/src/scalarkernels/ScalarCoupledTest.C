/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ScalarCoupledTest.h"
#include "Coupleable.h"

template <>
InputParameters
validParams<ScalarCoupledTest>()
{
  InputParameters params = validParams<ODEKernel>();
  params.addClassDescription("Banded coupling between scalar variables");
  params.addCoupledVar("c_up", "Up coupling");
  params.addCoupledVar("c_down", "Down coupling");
  return params;
}

ScalarCoupledTest::ScalarCoupledTest(const InputParameters & parameters)
  : ODEKernel(parameters),
    Coupleable(this),
    _up_coupling(isCoupledScalar("c_up")),
    _down_coupling(isCoupledScalar("c_down")),
    _c_up(_up_coupling ? coupledScalarValue("c_up") : _zero),
    _c_down(_down_coupling ? coupledScalarValue("c_down") : _zero)
{
}

Real
ScalarCoupledTest::computeQpResidual()
{
  return (_up_coupling ? _c_up[_i] - _u[_i] : 0.0) + (_down_coupling ? _c_down[_i] - _u[_i] : 0.0);
}

Real
ScalarCoupledTest::computeQpJacobian()
{
  return (_up_coupling ? -1.0 : 0.0) + (_down_coupling ? -1.0 : 0.0);
}
