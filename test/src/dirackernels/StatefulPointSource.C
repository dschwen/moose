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

#include "StatefulPointSource.h"

template <>
InputParameters
validParams<StatefulPointSource>()
{
  InputParameters params = validParams<DiracKernel>();
  return params;
}

StatefulPointSource::StatefulPointSource(const InputParameters & parameters)
  : DiracKernel(parameters),
    _value_old(getMaterialPropertyOld<Real>("thermal_conductivity")),
    _value(getMaterialProperty<Real>("thermal_conductivity"))
{
}

void
StatefulPointSource::addPoints()
{
  addPoint(Point(0.2, 0.3, 0));

  if (_t > 1.5)
    addPoint(Point(0.6, 0.8, 0));

  if (_t > 2.5)
    addPoint(Point(0.7, 0.8, 0));
}

Real
StatefulPointSource::computeQpResidual()
{
  _console << "Thermal conductivity =" << _value[_qp] << '\n'
           << "Thermal conductivity old =" << _value_old[_qp] << '\n';

  // This is negative because it's a forcing function that has been
  // brought over to the left side.
  return -_test[_i][_qp] * _value_old[_qp];
}
