//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DualRealOps.h"
#include "InputParameters.h"
#include "MooseTypes.h"
#include "NestedSolve.h"

class ConsoleStream;

/**
 * Base class that provides capability for Newton generalized (anisotropic) return mapping
 * iterations on a single variable
 */
template <bool is_ad>
class GeneralizedReturnMappingSolutionTempl
{
public:
  static InputParameters validParams();

  GeneralizedReturnMappingSolutionTempl(const InputParameters & parameters);
  virtual ~GeneralizedReturnMappingSolutionTempl() {}

protected:
  /**
   * Perform the return mapping iterations
   * @param effective_trial_stress Effective trial stress
   * @param scalar                 Inelastic strain increment magnitude being solved for
   * @param console                Console output
   */
  void returnMappingSolve(const GenericDenseVector<is_ad> & effective_trial_stress,
                          const GenericDenseVector<is_ad> & stress_new,
                          GenericReal<is_ad> & scalar,
                          const ConsoleStream & console);
  /**
   * Compute the minimum permissible value of the scalar.  For some models, the magnitude
   * of this may be known.
   * @param effective_trial_stress Effective trial stress
   */
  virtual GenericReal<is_ad>
  minimumPermissibleValue(const GenericDenseVector<is_ad> & effective_trial_stress) const;

  /**
   * Compute the maximum permissible value of the scalar.  For some models, the magnitude
   * of this may be known.
   * @param effective_trial_stress Effective trial stress
   */
  virtual GenericReal<is_ad>
  maximumPermissibleValue(const GenericDenseVector<is_ad> & effective_trial_stress) const;

  /**
   * Compute an initial guess for the value of the scalar. For some cases, an
   * intelligent starting point can provide enhanced robustness in the Newton
   * iterations. This is also an opportunity for classes that derive from this
   * to perform initialization tasks.
   * @param effective_trial_stress Effective trial stress
   */
  virtual GenericReal<is_ad>
  initialGuess(const GenericDenseVector<is_ad> & /*effective_trial_stress*/)
  {
    return 0.0;
  }

  /**
   * Compute the residual for a predicted value of the scalar.  This residual should be
   * in strain increment units for all models for consistency.
   * @param effective_trial_stress Effective trial stress
   * @param scalar                 Inelastic strain increment magnitude being solved for
   */
  virtual GenericReal<is_ad>
  computeResidual(const GenericDenseVector<is_ad> & effective_trial_stress,
                  const GenericDenseVector<is_ad> & stress_new,
                  const GenericReal<is_ad> & delta_gamma) = 0;

  /**
   * Compute a reference quantity to be used for checking relative convergence. This should
   * be in strain increment units for all models for consistency.
   * @param effective_trial_stress Effective trial stress
   * @param scalar                 Inelastic strain increment magnitude being solved for
   */
  virtual Real
  computeReferenceResidual(const GenericDenseVector<is_ad> & effective_trial_stress,
                           const GenericDenseVector<is_ad> & stress_new,
                           const GenericReal<is_ad> & residual,
                           const GenericReal<is_ad> & scalar_effective_inelastic_strain) = 0;

  virtual GenericReal<is_ad>
  computeDerivative(const GenericDenseVector<is_ad> & effective_trial_stress,
                    const GenericDenseVector<is_ad> & stress_new,
                    const GenericReal<is_ad> & scalar) = 0;
  /**
   * Finalize internal state variables for a model for a given iteration.
   * @param scalar                 Inelastic strain increment magnitude being solved for
   */
  virtual void iterationFinalize(GenericReal<is_ad> /*scalar*/) {}

  /// Whether to check to see whether iterative solution is within admissible range, and set within that range if outside
  bool _check_range;

private:
  enum class InternalSolveOutput
  {
    NEVER,
    ON_ERROR,
    ALWAYS
  } _internal_solve_output_on;

  enum class SolveState
  {
    SUCCESS,
    NAN_INF,
    EXCEEDED_ITERATIONS
  };

  ///@{ Residual values, kept as members to retain solver state for summary outputting
  GenericReal<is_ad> _initial_residual;
  GenericReal<is_ad> _residual;
  ///@}

  /// MOOSE input name of the object performing the solve
  const std::string _svrms_name;

  /**
   * Method called from within this class to perform the actual return mappping iterations.
   * @param effective_trial_stress Effective trial stress
   * @param scalar                 Inelastic strain increment magnitude being solved for
   * @param iter_output            Output stream -- if null, no output is produced
   * @return Whether the solution was successful
   */
  SolveState internalSolve(const GenericDenseVector<is_ad> & effective_trial_stress,
                           const GenericDenseVector<is_ad> & stress_new,
                           GenericReal<is_ad> & scalar,
                           std::stringstream * iter_output = nullptr);

  /// Solver helper object
  ADNestedSolve _solver;
};

typedef GeneralizedReturnMappingSolutionTempl<false> GeneralizedReturnMappingSolution;
typedef GeneralizedReturnMappingSolutionTempl<true> ADGeneralizedReturnMappingSolution;
