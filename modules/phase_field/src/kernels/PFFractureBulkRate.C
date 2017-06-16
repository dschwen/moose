/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "PFFractureBulkRate.h"
#include "MooseVariable.h"
#include "RankTwoTensor.h"
#include "MathUtils.h"

template <>
InputParameters
validParams<PFFractureBulkRate>()
{
  InputParameters params = validParams<PFFractureBulkRateBase>();
  params.addClassDescription(
      "Kernel to compute bulk energy contribution to damage order parameter residual equation");
  return params;
}

PFFractureBulkRate::PFFractureBulkRate(const InputParameters & parameters)
  : PFFractureBulkRateBase(parameters), _u_second(coupledSecond(_var.name()))
{
}

Real
PFFractureBulkRate::precomputeQpResidual()
{
  const Real & gc = _gc_prop[_qp];
  const Real & c = _u[_qp];
  const Real beta = _u_second[_qp].tr();
  const Real x = _width * beta + 2.0 * (1.0 - c) * _G0_pos[_qp] / gc - c / _width;

  return -MathUtils::positivePart(x) / _viscosity;
}

Real
PFFractureBulkRate::precomputeQpJacobian()
{
  const Real & gc = _gc_prop[_qp];
  const Real & c = _u[_qp];
  const Real beta = _u_second[_qp].tr();
  const Real x = _width * beta + 2.0 * (1.0 - c) * _G0_pos[_qp] / gc - c / _width;
  const Real dx = -2.0 * _G0_pos[_qp] / gc - 1.0 / _width;

  return -MathUtils::heavyside(x) / _viscosity * dx * _phi[_j][_qp];
}

Real
PFFractureBulkRate::computeQpOffDiagJacobian(unsigned int jvar)
{
  // bail out early if no stress derivative has been provided
  if (_dG0_pos_dstrain == NULL)
    return 0.0;

  // displacement variables
  unsigned int c_comp = 0;
  for (; c_comp < _ndisp; ++c_comp)
    if (jvar == _disp_var[c_comp])
      break;

  // Contribution of displacements to off-diagonal Jacobian of c
  if (c_comp < _ndisp)
  {
    const Real & c = _u[_qp];
    const Real & gc = _gc_prop[_qp];
    const Real beta = _u_second[_qp].tr();

    const Real x = _width * beta + 2.0 * (1.0 - c) * (_G0_pos[_qp] / gc) - c / _width;
    const Real xfac = -MathUtils::heavyside(x) / _viscosity * 2.0 * (1.0 - c) / gc;

    Real val = 0.0;
    for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
      val += ((*_dG0_pos_dstrain)[_qp](c_comp, i) + (*_dG0_pos_dstrain)[_qp](i, c_comp)) / 2.0 *
             _grad_phi[_j][_qp](i);

    return xfac * val * _test[_i][_qp];
  }

  return 0.0;
}
