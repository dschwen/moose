#include "InterfaceGradientMatch.h"

template<>
InputParameters validParams<InterfaceGradientMatch>()
{
  InputParameters params = validParams<InterfaceKernel>();
  params.addRequiredParam<unsigned int>("component", "Gradient component to set equal");
  return params;
}

InterfaceGradientMatch::InterfaceGradientMatch(const InputParameters & parameters) :
    InterfaceKernel(parameters),
    _component(getParam<unsigned int>("component"))
{
}

Real
InterfaceGradientMatch::computeQpResidual(Moose::DGResidualType type)
{
  // equal gradients means difference is zero
  Real res = _grad_u[_qp](_component) - _grad_u_neighbor[_qp](_component);

  switch (type)
  {
    case Moose::Element:
      return res * _test[_i][_qp];

    case Moose::Neighbor:
      return res * _test_neighbor[_i][_qp];
  }
}

Real
InterfaceGradientMatch::computeQpJacobian(Moose::DGJacobianType type)
{
  switch (type)
  {
    case Moose::ElementElement:
      return (_grad_phi[_j][_qp](_component) - _grad_u_neighbor[_qp](_component)) * _test[_i][_qp];

    case Moose::NeighborNeighbor:
      return (_grad_u[_qp](_component) - _grad_phi_neighbor[_j][_qp](_component)) * _test_neighbor[_i][_qp];

    case Moose::ElementNeighbor:
      return (_grad_u[_qp](_component) - _grad_phi_neighbor[_j][_qp](_component)) * _test[_i][_qp];

    case Moose::NeighborElement:
      return (_grad_phi[_j][_qp](_component) - _grad_u_neighbor[_qp](_component)) * _test_neighbor[_i][_qp];
  }
}
