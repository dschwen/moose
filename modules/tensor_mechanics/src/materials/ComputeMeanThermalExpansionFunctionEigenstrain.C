//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeMeanThermalExpansionFunctionEigenstrain.h"
#include "Function.h"

registerMooseObject("TensorMechanicsApp", ComputeMeanThermalExpansionFunctionEigenstrain);

InputParameters
ComputeMeanThermalExpansionFunctionEigenstrain::validParams()
{
  InputParameters params = ComputeThermalExpansionEigenstrainBase::validParams();
  params.addClassDescription("Computes eigenstrain due to thermal expansion using a function that "
                             "describes the mean thermal expansion as a function of temperature");
  params.addRequiredParam<FunctionName>(
      "thermal_expansion_function",
      "Function describing the mean thermal expansion as a function of temperature");
  params.addRequiredParam<Real>("thermal_expansion_function_reference_temperature",
                                "Reference temperature for thermal_expansion_function (IMPORTANT: "
                                "this is different in general from the stress_free_temperature)");
  params.addParam<Real>("epsilon",
                        0.0,
                        "Fallback finite differencing parameter for non differentiable thermal "
                        "expansion function points");

  return params;
}

ComputeMeanThermalExpansionFunctionEigenstrain::ComputeMeanThermalExpansionFunctionEigenstrain(
    const InputParameters & parameters)
  : ComputeMeanThermalExpansionEigenstrainBase(parameters),
    _thermal_expansion_function(getFunction("thermal_expansion_function")),
    _thexp_func_ref_temp(getParam<Real>("thermal_expansion_function_reference_temperature")),
    _epsilon(getParam<Real>("epsilon"))
{
}

Real
ComputeMeanThermalExpansionFunctionEigenstrain::referenceTemperature()
{
  return _thexp_func_ref_temp;
}

Real
ComputeMeanThermalExpansionFunctionEigenstrain::meanThermalExpansionCoefficient(
    const Real temperature)
{
  return _thermal_expansion_function.value(temperature, Point());
}

Real
ComputeMeanThermalExpansionFunctionEigenstrain::meanThermalExpansionCoefficientDerivative(
    const Real temperature)
{
  static const Point dummy;

  const auto dalpha_dT = _thermal_expansion_function.timeDerivative(temperature, dummy);

  // if the derivative is well formed or if the user did not specify a fallback
  // finite differencing parameter epsilon
  if (std::isnormal(dalpha_dT) || dalpha_dT == 0.0 || _epsilon == 0.0)
    return dalpha_dT;

  // if we end up here we are at a non-differentiable point in the thermal expansion
  // function (e.g. a node of a PiecewiseLinear function, which is guaranteed to be
  // reached if force_step_every_function_point = true in IterationAdaptiveDT)
  const auto alpha_left = _thermal_expansion_function.value(temperature - _epsilon / 2.0, dummy);
  const auto alpha_right = _thermal_expansion_function.value(temperature + _epsilon / 2.0, dummy);
  return (alpha_right - alpha_left) / _epsilon;
}
