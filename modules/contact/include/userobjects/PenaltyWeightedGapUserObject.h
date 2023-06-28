//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WeightedGapUserObject.h"
#include "AugmentedLagrangeInterface.h"

template <typename>
class MooseVariableFE;
class AugmentedLagrangianContactProblemInterface;

/**
 * User object for computing weighted gaps and contact pressure for penalty based
 * mortar constraints
 */
class PenaltyWeightedGapUserObject : virtual public WeightedGapUserObject,
                                     public AugmentedLagrangeInterface
{
public:
  static InputParameters validParams();

  PenaltyWeightedGapUserObject(const InputParameters & parameters);

  virtual void timestepSetup() override;

  virtual const ADVariableValue & contactPressure() const override;
  virtual void initialize() override;
  virtual void reinit() override;

  virtual Real getNormalContactPressure(const Node * const node) const override;
  virtual Real getNormalLagrangeMultiplier(const Node * const node) const;

  virtual Real getDeltaTangentialLagrangeMultiplier(const Node * const, const unsigned int) const
  {
    return 0.0;
  };
  virtual bool getActiveSetState(const Node * const node) const
  {
    return _active_set.count(_subproblem.mesh().nodePtr(node->id())) > 0;
  }

  virtual bool isAugmentedLagrangianConverged() override;
  virtual void augmentedLagrangianSetup() override;
  virtual void updateAugmentedLagrangianMultipliers() override;

protected:
  virtual const VariableTestValue & test() const override;
  virtual bool constrainedByOwner() const override { return false; }

  void selfInitialize();
  void selfTimestepSetup();

  /// The penalty factor
  const Real _penalty;

  /// penalty growth factor for augmented Lagrange
  const Real _penalty_multiplier;

  /// penetration tolerance for augmented Lagrange contact
  const Real _penetration_tolerance;

  /// The contact force on the mortar segument quadrature points
  ADVariableValue _contact_force;

  /// Map from degree of freedom to normal pressure for reporting
  std::unordered_map<const DofObject *, ADReal> _dof_to_normal_pressure;

  ///@{ augmented Lagrange problem and iteration number
  AugmentedLagrangianContactProblemInterface * const _augmented_lagrange_problem;
  const static unsigned int _no_iterations;
  const unsigned int & _lagrangian_iteration_number;
  ///@}

  /// Map from degree of freedom to augmented lagrange multiplier
  std::unordered_map<const DofObject *, Real> _dof_to_lagrange_multiplier;

  /// active set
  std::set<const DofObject *> _active_set;

  /// Map from degree of freedom to local penalty value
  std::unordered_map<const DofObject *, Real> _dof_to_local_penalty;

  /// Map from degree of freedom to previous AL iteration gap values
  std::unordered_map<const DofObject *, Real> _dof_to_previous_gap;

  /// scale factor for the linear prediction for the augmented lagrange multipliers
  const Real _predictor_scale;

  /// Current delta t... or timestep size.
  const Real & _dt;

  /// previous timestep size
  Real _dt_old;
};
