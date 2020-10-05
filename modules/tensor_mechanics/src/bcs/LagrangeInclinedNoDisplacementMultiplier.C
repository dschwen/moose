#include "LagrangeInclinedNoDisplacementMultiplier.h"

registerMooseObject("TensorMechanicsApp", LagrangeInclinedNoDisplacementMultiplier);

InputParameters
LagrangeInclinedNoDisplacementMultiplier::validParams()
{
  InputParameters params = ADIntegratedBC::validParams();
  params.addCoupledVar("displacements", "Displacement variables");
  return params;
}

LagrangeInclinedNoDisplacementMultiplier::LagrangeInclinedNoDisplacementMultiplier(
    const InputParameters & parameters)
  : ADIntegratedBC(parameters), _disp(adCoupledValues("displacements"))
{
}

ADReal
LagrangeInclinedNoDisplacementMultiplier::computeQpResidual()
{
  ADReal sum = 0.0;
  for (std::size_t i = 0; i < _disp.size(); ++i)
    sum += (*_disp[i])[_qp] * _normals[_qp](i);
  return _u[_qp] - sum;
}
