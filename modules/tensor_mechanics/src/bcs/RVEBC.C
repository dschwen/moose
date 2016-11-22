/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#include "RVEBC.h"
#include "RankTwoTensor.h"
#include "RankFourTensor.h"
#include "ElasticityTensorTools.h"

template<>
InputParameters validParams<RVEBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<unsigned int>("component", "The displacement component corresponding the variable this BC acts on.");
  return params;
}

RVEBC::RVEBC(const InputParameters & parameters) :
    IntegratedBC(parameters),
    _component(getParam<unsigned int>("component")),
    _stress(getMaterialPropertyByName<RankTwoTensor>("stress")),
    _Jacobian_mult(getMaterialPropertyByName<RankFourTensor>("Jacobian_mult")),
    _elasticity_tensor(getMaterialProperty<RankFourTensor>("elasticity_tensor"))
{
}

Real
RVEBC::computeQpResidual()
{
  return _test[_i][_qp] * _stress[_qp].row(_component) * _normals[_qp];
}

Real
RVEBC::computeQpJacobian()
{
  return 0.0;
}

// Real
// RVEBC::computeQpOffDiagJacobian(unsigned int jvar)
// {
//   return ElasticityTensorTools::momentJacobian(_elasticity_tensor[_qp], _component, _component, _test[_i][_qp], _grad_phi[_j][_qp]);
// }
