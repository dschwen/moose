//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementUserObject.h"
#include "GlobalStrainUserObjectInterface.h"

#include "RankTwoTensor.h"
#include "RankFourTensor.h"

class GlobalStrainUserObject : public ElementUserObject, public GlobalStrainUserObjectInterface
{
public:
  static InputParameters validParams();

  GlobalStrainUserObject(const InputParameters & parameters);

  void initialize() override;
  void execute() override;
  void threadJoin(const UserObject & uo) override;
  void finalize() override;
  virtual const RankTwoTensor & getResidual() const override;
  virtual const RankFourTensor & getJacobian() const override;
  virtual const VectorValue<bool> & getActiveDirections() const override;
  virtual bool hasExplicitActiveDirections() const override;

  /**
   * Calculate additional applied stresses
   */
  virtual void computeAdditionalStress() {};

protected:
  /// Base name of the material system
  const std::string _base_name;

  const MaterialProperty<RankFourTensor> & _dstress_dstrain;

  /// The stress tensor
  const MaterialProperty<RankTwoTensor> & _stress;

  RankTwoTensor _applied_stress_tensor;
  RankTwoTensor _residual;
  RankFourTensor _jacobian;

  const unsigned int _dim;

  /// Displacement variables
  const std::vector<const MooseVariableFieldBase *> _disp_var;

  /// Whether directions were explicitly selected rather than inferred from periodicity
  const bool _has_explicit_active_directions;

  /// Directions with active global strain components
  VectorValue<bool> _active_dir;
};
