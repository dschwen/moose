//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADPARSEDMATERIALBASE_H
#define ADPARSEDMATERIALBASE_H

#include "ADMaterial.h"
#include "DerivativeMaterialPropertyNameInterface.h"
#include "ExprTkWrapper.h"

#include <string>
#include <vector>

#define usingParsedMaterialBaseMembers                                                             \
  usingMaterialMembers;                                                                            \
  usingDerivativeMaterialPropertyNameInterfaceMembers;                                             \
  using ADParsedMaterialBase<compute_stage>::_function;                                            \
  using ADParsedMaterialBase<compute_stage>::_args;                                                \
  using ADParsedMaterialBase<compute_stage>::_var_names;                                           \
  using ADParsedMaterialBase<compute_stage>::_f_name;                                              \
  using ADParsedMaterialBase<compute_stage>::_prop_F

// Forward Declarations
template <ComputeStage>
class ADParsedMaterialBase;
template <typename>
class ExprTkWrapper;

declareADValidParams(ADParsedMaterialBase);

/**
 * Parsed material using ExprTk
 */
template <ComputeStage compute_stage>
class ADParsedMaterialBase : public ADMaterial<compute_stage>,
                             public DerivativeMaterialPropertyNameInterface
{
public:
  ADParsedMaterialBase(const InputParameters & parameters);

protected:
  /// function expression to parse
  const std::string _function;

  /// Coupled variable value for the order parameter
  std::vector<const ADVariableValue *> _args;

  /**
   * Names under which the coupled variables appear in the function expression.
   * If omitted they appear under their original MOOSE variable names.
   */
  std::vector<std::string> _var_names;

  /// function value property name
  const MaterialPropertyName _f_name;

  /// function value
  ADMaterialProperty(Real) & _prop_F;

  usingMaterialMembers;
};

#endif // ADPARSEDMATERIALBASE_H
