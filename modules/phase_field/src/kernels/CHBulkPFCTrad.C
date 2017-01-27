/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CHBulkPFCTrad.h"

template<>
InputParameters validParams<CHBulkPFCTrad>()
{
  InputParameters params = CHBulk<Real>::validParams();
  return params;
}

CHBulkPFCTrad::CHBulkPFCTrad(const InputParameters & parameters) :
    CHBulk<Real>(parameters),
    _C0(getMaterialProperty<Real>("C0")),
    _a(getMaterialProperty<Real>("a")),
    _b(getMaterialProperty<Real>("b"))
{
}

RealGradient
CHBulkPFCTrad::computeGradDFDCons(PFFunctionType type)
{
  const Real d2fdc2 = 1.0 - _C0[_qp] - _a[_qp] * _u[_qp] + _b[_qp] * _u[_qp] * _u[_qp];

  switch (type)
  {
    case Residual:
      return d2fdc2 * _grad_u[_qp];

    case Jacobian:
    {
      const Real d3fdc3 = - _a[_qp] + 2.0 * _b[_qp] * _u[_qp];
      return d2fdc2 * _grad_phi[_j][_qp] + d3fdc3 * _grad_u[_qp] * _phi[_j][_qp];
    }
  }

  mooseError("Invalid type passed in");
}
