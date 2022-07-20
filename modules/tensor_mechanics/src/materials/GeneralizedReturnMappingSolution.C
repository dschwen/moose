//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeneralizedReturnMappingSolution.h"

#include "Moose.h"
#include "MooseEnum.h"
#include "MooseObject.h"
#include "ConsoleStreamInterface.h"
#include "Conversion.h"
#include "MathUtils.h"

#include "DualRealOps.h"

#include <limits>
#include <string>
#include <cmath>
#include <memory>

template <bool is_ad>
InputParameters
GeneralizedReturnMappingSolutionTempl<is_ad>::validParams()
{
  InputParameters params = ADNestedSolve::validParams();
  params.set<unsigned int>("min_iterations") = 1;
  return params;
}
template <bool is_ad>
GeneralizedReturnMappingSolutionTempl<is_ad>::GeneralizedReturnMappingSolutionTempl(
    const InputParameters & parameters)
  : _check_range(false),
    _initial_residual(0.0),
    _residual(0.0),
    _svrms_name(parameters.get<std::string>("_object_name")),
    _solver(parameters)
{
}

template <bool is_ad>
GenericReal<is_ad>
GeneralizedReturnMappingSolutionTempl<is_ad>::minimumPermissibleValue(
    const GenericDenseVector<is_ad> & /*effective_trial_stress*/) const
{
  return std::numeric_limits<Real>::lowest();
}

template <bool is_ad>
GenericReal<is_ad>
GeneralizedReturnMappingSolutionTempl<is_ad>::maximumPermissibleValue(
    const GenericDenseVector<is_ad> & /*effective_trial_stress*/) const
{
  return std::numeric_limits<Real>::max();
}

template <bool is_ad>
void
GeneralizedReturnMappingSolutionTempl<is_ad>::returnMappingSolve(
    const GenericDenseVector<is_ad> & stress_dev,
    const GenericDenseVector<is_ad> & stress_new,
    GenericReal<is_ad> & scalar)
{
  // do the internal solve
  auto solve_state = internalSolve(stress_dev, stress_new, scalar);

  if (solve_state != SolveState::SUCCESS &&
      _internal_solve_output_on != InternalSolveOutput::ALWAYS)
  {
    // output suppressed by user, throw immediately
    if (_internal_solve_output_on == InternalSolveOutput::NEVER)
      mooseException("");

    // user expects some kind of output, if necessary setup output stream now
    // if (!iter_output)
    auto iter_output = std::make_unique<std::stringstream>();

    // add the appropriate error message to the output
    switch (solve_state)
    {
      case SolveState::NAN_INF:
        mooseException("Encountered inf or nan in material return mapping iterations.\n");
        break;

      case SolveState::EXCEEDED_ITERATIONS:
        mooseException("Exceeded maximum iterations in material return mapping iterations.\n");
        break;

      default:
        mooseError("Unhandled solver state");
    }
  }
}

template <bool is_ad>
typename GeneralizedReturnMappingSolutionTempl<is_ad>::SolveState
GeneralizedReturnMappingSolutionTempl<is_ad>::internalSolve(
    const GenericDenseVector<is_ad> & stress_dev,
    const GenericDenseVector<is_ad> & stress_new,
    GenericReal<is_ad> & delta_gamma)
{
  auto residualFunctor = [&](const ADReal & guess, ADReal & residual)
  { residual = computeResidual(stress_dev, stress_new, guess); };
  auto jacobianFunctor = [&](const ADReal & guess, ADReal & jacobian)
  { jacobian = computeDerivative(stress_dev, stress_new, guess); };
  auto boundsFunctor = [&]()
  {
    return std::make_pair(MetaPhysicL::raw_value(minimumPermissibleValue(stress_dev)),
                          MetaPhysicL::raw_value(maximumPermissibleValue(stress_dev)));
  };

  delta_gamma = initialGuess(stress_dev);

  if (_check_range)
    _solver.nonlinearBounded(delta_gamma, residualFunctor, jacobianFunctor, boundsFunctor);
  else
    _solver.nonlinear(delta_gamma, residualFunctor, jacobianFunctor);

  if (std::isnan(delta_gamma) || std::isinf(MetaPhysicL::raw_value(delta_gamma)))
    return SolveState::NAN_INF;

  if (_solver.getState() == ADNestedSolve::State::NOT_CONVERGED)
    return SolveState::EXCEEDED_ITERATIONS;

  return SolveState::SUCCESS;
}

template class GeneralizedReturnMappingSolutionTempl<false>;
template class GeneralizedReturnMappingSolutionTempl<true>;
