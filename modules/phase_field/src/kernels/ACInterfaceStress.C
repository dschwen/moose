/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ACInterfaceStress.h"

template <>
InputParameters
validParams<ACInterfaceStress>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Interface stress driving force Allen-Cahn Kernel");
  params.addParam<MaterialPropertyName>("mob_name", "L", "The mobility used with the kernel");
  return params;
}

ACInterfaceStress::ACInterfaceStress(const InputParameters & parameters)
  : DerivativeMaterialInterface<JvarMapKernelInterface<Kernel>>(parameters),
    _L(getMaterialProperty<Real>("mob_name"))
{
}

Real
ACInterfaceStress::computeQpResidual()
{
  // no interface, return zero stress
  const Real grad_norm_sq = _grad_v[_qp].norm_sq();
  if (grad_norm_sq < libMesh::TOLERANCE)
  {
    S.zero();
    return;
  }

  const grad_norm = std::sqrt(grad_norm_sq);

  const Real nx = _grad_v[_qp](0);
  const Real ny = _grad_v[_qp](1);
  const Real nz = _grad_v[_qp](2);

  const Real s = _stress / grad_norm;
  const Real ds = -_stress / (grad_norm * grad_norm_sq);
  const Real dsx = ds * nx;
  const Real dsy = ds * ny;
  const Real dsz = ds * nz;

  S(0, 0) = (ny * ny + nz * nz) * s;
  S(1, 0) = S(0, 1) = -nx * ny * s;
  S(1, 1) = (nx * nx + nz * nz) * s;
  S(2, 0) = S(0, 2) = -nx * nz * s;
  S(2, 1) = S(1, 2) = -ny * nz * s;
  S(2, 2) = (nx * nx + ny * ny) * s;

  // d/d(grad eta)_x
  _dS(0, 0, 0) = (ny * ny + nz * nz) * dsx;                // (ny * ny + nz * nz) * s;
  _dS(0, 1, 0) = _dS(0, 0, 1) = -ny * s - nx * ny * dsx;   // -nx * ny * s;
  _dS(0, 1, 1) = 2.0 * nx * s + (nx * nx + nz * nz) * dsx; // (nx * nx + nz * nz) * s;
  _dS(0, 2, 0) = _dS(0, 0, 2) = -nz * s - nx * nz * dsx;   // -nx * nz * s;
  _dS(0, 2, 1) = _dS(0, 1, 2) = -ny * nz * dsx;            // -ny * nz * s;
  _dS(0, 2, 2) = 2.0 * nx * s + (nx * nx + ny * ny) * dsx; // (nx * nx + ny * ny) * s;

  // d/d(grad eta)_y
  _dS(1, 0, 0) = 2.0 * ny * s + (ny * ny + nz * nz) * dsy; // (ny * ny + nz * nz) * s;
  _dS(1, 1, 0) = _dS(1, 0, 1) = -nx * s - nx * ny * dsy;   // -nx * ny * s;
  _dS(1, 1, 1) = (nx * nx + nz * nz) * dsy;                // (nx * nx + nz * nz) * s;
  _dS(1, 2, 0) = _dS(1, 0, 2) = -nx * nz * dsy;            // -nx * nz * s;
  _dS(1, 2, 1) = _dS(1, 1, 2) = -nz * s - ny * nz * dsy;   // -ny * nz * s;
  _dS(1, 2, 2) = 2.0 * ny * s + (nx * nx + ny * ny) * dsy; // (nx * nx + ny * ny) * s;

  // d/d(grad eta)_z
  _dS(2, 0, 0) = 2.0 * nz * s + (ny * ny + nz * nz) * dsz; // (ny * ny + nz * nz) * s;
  _dS(2, 1, 0) = _dS(2, 0, 1) = -nx * ny * dsz;            // -nx * ny * s;
  _dS(2, 1, 1) = 2.0 * ny * s + (nx * nx + nz * nz) * dsz; // (nx * nx + nz * nz) * s;
  _dS(2, 2, 0) = _dS(2, 0, 2) = -nx * s - nx * nz * dsz;   // -nx * nz * s;
  _dS(2, 2, 1) = _dS(2, 1, 2) = -ny * s - ny * nz * dsz;   // -ny * nz * s;
  _dS(2, 2, 2) = (nx * nx + ny * ny) * dsz;                // (nx * nx + ny * ny) * s;

  return 0.0;
}

Real
ACInterfaceStress::computeQpJacobian()
{
  return 0.0;
}

Real
ACInterfaceStress::computeQpOffDiagJacobian(unsigned int jvar)
{
  return 0.0;
}
