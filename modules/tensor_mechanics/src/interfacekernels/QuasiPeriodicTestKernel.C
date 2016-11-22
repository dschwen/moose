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
#include <iostream>
template<>
InputParameters validParams<QuasiPeriodicTestKernel>()
{
  InputParameters params = validParams<InterfaceKernel>();
  params.addRequiredParam<unsigned int>("component", "The gradient component");
  params.addRequiredParam<Real>("penalty", "An artificial penalty method for the interface kernel");
  params.addRequiredParam<Real>("D", "");
  params.addRequiredParam<Real>("D_neighbor", "");
  return params;
}

QuasiPeriodicTestKernel::QuasiPeriodicTestKernel(const InputParameters & parameters) :
    InterfaceKernel(parameters),
    _component(getParam<unsigned int>("component")),
    _penalty(getParam<Real>("penalty")),
    _D(getParam<Real>("D")),
    _D_neighbor(getParam<Real>("D_neighbor"))
{
}

Real
QuasiPeriodicTestKernel::computeQpResidual(Moose::DGResidualType type)
{
  Real r = _penalty * (_D * _u[_qp] -  _D_neighbor * _neighbor_value[_qp] );
  switch (type)
  {
    case Moose::Element:
    {
      r *= _test[_i][_qp];
      //std::cout << "r = " << r << '\n';
    }
      break;

    case Moose::Neighbor:
    {
      r *= - _test_neighbor[_i][_qp];
      //std::cout << "r = " << r << '\n';
    }
      break;
  }

  return r;
}

Real
QuasiPeriodicTestKernel::computeQpJacobian(Moose::DGJacobianType type)
{
  Real jac;

  switch (type)
  {
    case Moose::ElementElement:
      jac = _penalty * _D * _phi[_j][_qp]  * _test[_i][_qp];
      break;

    case Moose::NeighborNeighbor:
      jac = _penalty * _D_neighbor * _phi_neighbor[_j][_qp]  * _test_neighbor[_i][_qp];
      break;

    case Moose::NeighborElement:
      jac = - _penalty * _D * _phi[_j][_qp] * _test_neighbor[_i][_qp];
      break;

    case Moose::ElementNeighbor:
      jac = - _penalty * _D_neighbor * _phi_neighbor[_j][_qp]  * _test[_i][_qp];
      break;
  }

  return jac;
}
