#include "HHPFCRFF.h"

template<>
InputParameters validParams<HHPFCRFF>()
{
  InputParameters params = validParams<KernelValue>();
  params.addCoupledVar("coupled_var", "The name of the coupled variable, if one is used in the kernel");
  params.addRequiredParam<MaterialPropertyName>("prop_name", "Name of material property to be used in the kernel");
  params.addRequiredParam<bool>("positive", "If the kernel is positive, this is true, if negative, it is false");
  return params;
}

HHPFCRFF::HHPFCRFF(const std::string & name, InputParameters parameters) :
    KernelValue(name, parameters),
    _kernel_sign(getParam<bool>("positive") ? 1.0 : -1.0),
    _prop(getMaterialProperty<Real>("prop_name")),
    _coupled(isCoupled("coupled_var") ? &coupledValue("coupled_var") : NULL),
    _coupled_var(_has_coupled_var ? coupled("coupled_var") : 0)
{
}

Real
HHPFCRFF::precomputeQpResidual()
{
  // Assign value of the variable, whether coupled or not
  Real var = _coupled ? (*_coupled)[_qp] : _u[_qp];
  return _kernel_sign * _prop[_qp] * var;
}

Real
HHPFCRFF::precomputeQpJacobian()
{
  return _coupled ? 0.0 : _kernel_sign * _prop[_qp] * _phi[_j][_qp];
}

Real
HHPFCRFF::computeQpOffDiagJacobian(unsigned int jvar)
{
  if (_coupled && jvar == _coupled_var)
    return _kernel_sign * _prop[_qp] * _phi[_j][_qp] * _test[_i][_qp];

  return 0.0;
}
