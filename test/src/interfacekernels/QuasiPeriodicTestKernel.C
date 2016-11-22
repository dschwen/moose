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

#include "QuasiPeriodicTestKernel.h"

#include <cmath>

template<>
InputParameters validParams<QuasiPeriodicTestKernel>()
{
  InputParameters params = validParams<InterfaceKernel>();
  params.addParam<Real>("D", 1.0, "The diffusion coefficient.");
  params.addParam<Real>("D_neighbor", 1.0, "The neighboring diffusion coefficient.");
  params.addParam<Real>("offset", 0.0, "The offset of the problem");
  return params;
}

QuasiPeriodicTestKernel::QuasiPeriodicTestKernel(const InputParameters & parameters) :
    InterfaceKernel(parameters),
    _D(getParam<Real>("D")),
    _D_neighbor(getParam<Real>("D_neighbor")),
    _offset(getParam<Real>("offset"))
{
  if (!parameters.isParamValid("boundary"))
  {
    mooseError("In order to use the InterfaceDiffusion dgkernel, you must specify a boundary where it will live.");
  }
}

Real
QuasiPeriodicTestKernel::computeQpResidual(Moose::DGResidualType type)
{
  Real r = 0.5 * (_D * _u[_qp]  - _D_neighbor *_neighbor_value[_qp] + _offset);

  switch (type)
  {
  case Moose::Element:
    r *= _test[_i][_qp];
    break;

  case Moose::Neighbor:
    r *= _test_neighbor[_i][_qp];
    break;
  }

  return r;
}

Real
QuasiPeriodicTestKernel::computeQpJacobian(Moose::DGJacobianType type)
{
  Real jac = 0;

  switch (type)
  {

    case Moose::ElementElement:
      jac += 0.5 * _D * _phi[_j][_qp] * _test[_i][_qp];
      break;

    case Moose::NeighborNeighbor:
      jac -= 0.5 * _D_neighbor * _phi_neighbor[_j][_qp] * _test_neighbor[_i][_qp];
      break;

    case Moose::NeighborElement:
      jac -= 0.5 * _D * _phi[_j][_qp] * _test_neighbor[_i][_qp];
      break;

    case Moose::ElementNeighbor:
      jac +=  0.5 * _D_neighbor * _phi_neighbor[_j][_qp]  * _test[_i][_qp];
      break;
  }

  return jac;
}
