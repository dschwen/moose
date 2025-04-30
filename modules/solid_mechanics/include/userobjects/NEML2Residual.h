//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include <ATen/core/TensorBody.h>
#ifdef NEML2_ENABLED

#include "GeneralUserObject.h"
#include "neml2/tensors/Tensor.h"

// System includes
#include <string>

// Forward declarations
class InputParameters;
class FEProblemBase;

/**
 * A base class that can be used for executioners solving for steady state problems.
 */
class NEML2Residual : public GeneralUserObject
{
public:
  static InputParameters validParams();

  NEML2Residual(const InputParameters & parameters);

  virtual void initialSetup() override;
  virtual void meshChanged() override;

  virtual void initialize() override final {}
  virtual void execute() override;
  virtual void finalize() override final {}

protected:
  torch::TensorOptions _options;

  /// coupled variables to provide the DOF values
  std::vector<NonlinearVariableName> _variable_names;

  /// Variables
  std::vector<const MooseVariableFieldBase *> _variables;

  /// local DOF indices for each element, variable, and node
  std::vector<int64_t> _connectivity;
  at::Tensor _connectivity_tensor;

  /// Solution vector
  const NumericVector<Number> * const & _solution;

  at::Tensor _solution_tensor;
  int64_t _solution_size;

  /// Residual vector
  PetscVector<Real> * const _residual;
  at::Tensor _residual_tensor;
};

#endif
