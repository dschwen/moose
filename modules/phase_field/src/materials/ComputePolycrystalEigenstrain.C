/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "ComputePolycrystalEigenstrain.h"
#include "RankTwoTensor.h"

template <>
InputParameters
validParams<ComputePolycrystalEigenstrain>()
{
  InputParameters params = validParams<ComputeEigenstrainBase>();
  params.addClassDescription("Compute an evolving eigenstrain tensor coupled to a grain growth phase field model.");
  params.addRequiredParam<UserObjectName>("grain_tracker", "Name of GrainTracker user object that provides RankTwoTensors");
  params.addParam<Real>("length_scale", 1.0e-9, "Lengthscale of the problem, in meters");
  params.addParam<Real>("pressure_scale", 1.0e6, "Pressure scale of the problem, in pa");
  params.addRequiredCoupledVarWithAutoBuild("v", "var_name_base", "op_num", "Array of coupled variables");
  return params;
}

ComputePolycrystalEigenstrain::ComputePolycrystalEigenstrain(const InputParameters & parameters)
  : DerivativeMaterialInterface<ComputeEigenstrainBase>(parameters),
    _grain_tracker(getUserObject<GrainDataTracker<RankTwoTensor>>("grain_tracker")),
    _op_num(coupledComponents("v")),
    _vals(_op_num),
    _Deigenstrain(_op_num)
{
  // Loop over variables (ops)
  for (auto op_index = beginIndex(_vals); op_index < _op_num; ++op_index)
  {
    // Initialize variables
    _vals[op_index] = &coupledValue("v", op_index);

    // declare elasticity tensor derivative properties
    _Deigenstrain[op_index] = &declarePropertyDerivative<RankTwoTensor>(_base_name + "elastic_strain", getVar("v", op_index)->name());
  }
}

void
ComputePolycrystalEigenstrain::computeQpEigenstrain()
{
  // Get list of active order parameters from grain tracker
  const auto & op_to_grains = _grain_tracker.getVarToFeatureVector(_current_elem->id());

  // Calculate elasticity tensor
  _eigenstrain[_qp].zero();
  Real sum_h = 0.0;
  for (auto op_index = beginIndex(op_to_grains); op_index < op_to_grains.size(); ++op_index)
  {
    auto grain_id = op_to_grains[op_index];
    if (grain_id == FeatureFloodCount::invalid_id)
      continue;

    // Interpolation factor for eigenstrain tensors
    Real h = (1.0 + std::sin(libMesh::pi * ((*_vals[op_index])[_qp] - 0.5))) / 2.0;

    // Sum all rotated elasticity tensors
    _eigenstrain[_qp] += _grain_tracker.getData(grain_id) * h;
    sum_h += h;
  }

  const Real tol = 1.0e-10;
  sum_h = std::max(sum_h, tol);
  _eigenstrain[_qp] /= sum_h;

  // Calculate eigenstrain tensor derivative
  for (auto op_index = beginIndex(op_to_grains); op_index < op_to_grains.size(); ++op_index)
  {
    auto grain_id = op_to_grains[op_index];
    if (grain_id == FeatureFloodCount::invalid_id)
    {
      (*_Deigenstrain[op_index])[_qp].zero();
      continue;
    }

    const Real dhDopi = libMesh::pi * std::cos(libMesh::pi * ((*_vals[op_index])[_qp] - 0.5)) / 2.0;
    (*_Deigenstrain[op_index])[_qp] = -(_grain_tracker.getData(grain_id) - _eigenstrain[_qp]) * dhDopi / sum_h;
  }
}
