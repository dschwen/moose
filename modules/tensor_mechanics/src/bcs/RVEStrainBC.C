/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "RVEStrainBC.h"

template<>
InputParameters validParams<RVEStrainBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<unsigned int>("component", "The displacement component corresponding the variable this BC acts on.");
  return params;
}

RVEStrainBC::RVEStrainBC(const InputParameters & parameters) :
    IntegratedBC(parameters),
    _component(getParam<unsigned int>("component"))
{
}

Real
RVEStrainBC::computeQpResidual()
{
  return - _test[_i][_qp]*_grad_u[_qp](_component);
}

Real
RVEStrainBC::computeQpJacobian()
{
  return - _test[_i][_qp]*_grad_phi[_j][_qp](_component);
}
