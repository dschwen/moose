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

#include "RVEPeriodicStrain.h"

#include "RankTwoTensor.h"
#include "ElasticityTensorTools.h"
#include "MooseMesh.h"
#include <cmath>

template<>
InputParameters validParams<RVEPeriodicStrain>()
{
  InputParameters params = validParams<InterfaceKernel>();
  params.addRequiredParam<unsigned int>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addRequiredParam<unsigned int>("direction", "An integer corresponding to the direction the derivative is taken. (0 for x, 1 for y, 2 for z)");
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addRequiredCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.set<std::string>("_moose_base") = "InterfaceKernel";
  params.addRequiredParam<Real>("penalty", "Introduce a penalty for this kernel");
  return params;
}

RVEPeriodicStrain::RVEPeriodicStrain(const InputParameters & parameters) :
    InterfaceKernel(parameters),
    _component(getParam<unsigned int>("component")),
    _direction(getParam<unsigned int>("direction")),
    _elasticity_tensor(getMaterialProperty<RankFourTensor>("elasticity_tensor")),
    _disp_x_var(coupled("disp_x")),
    _disp_y_var(coupled("disp_y")),
    _disp_z_var(_mesh.dimension() == 3 ? coupled("disp_z") : libMesh::invalid_uint),
    _disp_x(coupledValue("disp_x")),
    _disp_y(coupledValue("disp_y")),
    _disp_z(_mesh.dimension() == 3 ? coupledValue("disp_z") : _zero),
    _grad_disp_x(coupledGradient("disp_x")),
    _grad_disp_y(coupledGradient("disp_y")),
    _grad_disp_z(_mesh.dimension() == 3 ? coupledGradient("disp_z") : _grad_zero),
    _disp_x_var_test(*getVar("disp_x", 0)),
    _disp_y_var_test(*getVar("disp_y", 0)),
    _disp_z_var_test(*getVar("disp_z", 0)), //need to fix this so that we can go to 2D
    _grad_disp_x_neighbor_value(_disp_x_var_test.gradSlnNeighbor()),
    _grad_disp_y_neighbor_value(_disp_y_var_test.gradSlnNeighbor()),
    _grad_disp_z_neighbor_value(_mesh.dimension() == 3 ? _disp_z_var_test.gradSlnNeighbor() : _grad_zero),
    _penalty(getParam<Real>("penalty"))
{
  if (!parameters.isParamValid("boundary"))
  {
    mooseError("In order to use the RVEPeriodicStrain interface kernel, you must specify a boundary where it will live.");
  }
}

Real
RVEPeriodicStrain::computeQpResidual(Moose::DGResidualType type)
{
  Real r = 0.0;
  if (_component == 0)
   r += _penalty * (_grad_disp_x[_qp](_direction) - _grad_disp_x_neighbor_value[_qp](_direction));
  else if (_component == 1)
   r += _penalty * (_grad_disp_y[_qp](_direction) - _grad_disp_y_neighbor_value[_qp](_direction));
  else if (_component == 2)
   r += _penalty * (_grad_disp_z[_qp](_direction) - _grad_disp_z_neighbor_value[_qp](_direction));

  switch (type)
  {
    case Moose::Element:
      r *= _test[_i][_qp];
      break;
    case Moose::Neighbor:
      r *= - _test_neighbor[_i][_qp];
      break;
  }
  return r;
}

Real
RVEPeriodicStrain::computeQpJacobian(Moose::DGJacobianType type)
{
  Real jac = 0.0;
  switch (type)
  {
    case Moose::ElementElement:
      jac += _penalty * _grad_phi[_j][_qp](_direction) * _test[_i][_qp];
      break;

    case Moose::NeighborNeighbor:
      jac += _penalty * _grad_phi_neighbor[_j][_qp](_direction) * _test_neighbor[_i][_qp];
      break;

    case Moose::NeighborElement:
      jac += - _penalty *  _grad_phi[_j][_qp](_direction) * _test_neighbor[_i][_qp];
      break;

    case Moose::ElementNeighbor:
      jac +=  - _penalty * _grad_phi_neighbor[_j][_qp](_direction)  * _test[_i][_qp];
      break;
  }
  return jac;
}
