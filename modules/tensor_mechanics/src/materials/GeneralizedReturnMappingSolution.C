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
  // params.addParam<Real>(
  //     "relative_tolerance", 1e-8, "Relative convergence tolerance for Newton iteration");
  // params.addParam<Real>(
  //     "absolute_tolerance", 1e-11, "Absolute convergence tolerance for Newton iteration");
  // params.addParam<Real>("acceptable_multiplier",
  //                       10,
  //                       "Factor applied to relative and absolute "
  //                       "tolerance for acceptable convergence if "
  //                       "iterations are no longer making progress");
  //
  // // diagnostic output parameters
  // MooseEnum internal_solve_output_on_enum("never on_error always", "on_error");
  // params.addParam<MooseEnum>("internal_solve_output_on",
  //                            internal_solve_output_on_enum,
  //                            "When to output internal Newton solve information");
  // params.addParam<bool>("internal_solve_full_iteration_history",
  //                       false,
  //                       "Set true to output full internal Newton iteration history at times "
  //                       "determined by `internal_solve_output_on`. If false, only a summary is "
  //                       "output.");
  // params.addParamNamesToGroup("internal_solve_output_on internal_solve_full_iteration_history",
  //                             "Debug");
  return params;
}
template <bool is_ad>
GeneralizedReturnMappingSolutionTempl<is_ad>::GeneralizedReturnMappingSolutionTempl(
    const InputParameters & parameters)
  : // _check_range(false),
    //   _line_search(true),
    //   _bracket_solution(false),
    //   _internal_solve_output_on(
    //       parameters.get<MooseEnum>("internal_solve_output_on").getEnum<InternalSolveOutput>()),
    // _max_its(50),
    // _internal_solve_full_iteration_history(
    //     parameters.get<bool>("internal_solve_full_iteration_history")),
    // _relative_tolerance(parameters.get<Real>("relative_tolerance")),
    // _absolute_tolerance(parameters.get<Real>("absolute_tolerance")),
    // _acceptable_multiplier(parameters.get<Real>("acceptable_multiplier")),
    // _num_resids(30),
    // _residual_history(_num_resids, std::numeric_limits<Real>::max()),
    // _iteration(0),
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
    GenericReal<is_ad> & scalar,
    const ConsoleStream & console)
{
  // do the internal solve and capture iteration info during the first round
  // iff full history output is requested regardless of whether the solve failed or succeeded
  auto solve_state = internalSolve(stress_dev, stress_new, scalar, nullptr);

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
        *iter_output << "Encountered inf or nan in material return mapping iterations.\n";
        break;

      case SolveState::EXCEEDED_ITERATIONS:
        *iter_output << "Exceeded maximum iterations in material return mapping iterations.\n";
        break;

      default:
        mooseError("Unhandled solver state");
    }

    // // if full history output is only requested for failed solves we have to repeat
    // // the solve a second time
    // if (_internal_solve_full_iteration_history)
    //   internalSolve(stress_dev, stress_new, scalar, iter_output.get());
    //
    // // Append summary and throw exception
    // outputIterationSummary(iter_output.get(), _iteration);
    // mooseException(iter_output->str());
  }

  // if (_internal_solve_output_on == InternalSolveOutput::ALWAYS)
  // {
  //   // the solve did not fail but the user requested debug output anyways
  //   outputIterationSummary(iter_output.get(), _iteration);
  //   console << iter_output->str();
  // }
}

template <bool is_ad>
typename GeneralizedReturnMappingSolutionTempl<is_ad>::SolveState
GeneralizedReturnMappingSolutionTempl<is_ad>::internalSolve(
    const GenericDenseVector<is_ad> & stress_dev,
    const GenericDenseVector<is_ad> & stress_new,
    GenericReal<is_ad> & delta_gamma,
    std::stringstream * iter_output)
{
  auto residualFunctor = [&](const ADReal & guess, ADReal & residual)
  { residual = computeResidual(stress_dev, stress_new, guess); };
  auto jacobianFunctor = [&](const ADReal & guess, ADReal & jacobian)
  { jacobian = computeDerivative(stress_dev, stress_new, guess); };

  delta_gamma = initialGuess(stress_dev);
  _solver.nonlinear(delta_gamma, residualFunctor, jacobianFunctor);

  // return SolveState::SUCCESS;
  // ++_iteration;
  //
  // if (std::isnan(_residual) || std::isinf(MetaPhysicL::raw_value(_residual)))
  //   return SolveState::NAN_INF;
  //
  // if (_iteration == _max_its)
  //   return SolveState::EXCEEDED_ITERATIONS;

  return SolveState::SUCCESS;
}

template class GeneralizedReturnMappingSolutionTempl<false>;
template class GeneralizedReturnMappingSolutionTempl<true>;
