//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADDERIVATIVEPARSEDMATERIAL_H
#define ADDERIVATIVEPARSEDMATERIAL_H

#include "ADParsedMaterialBase.h"

// Forward Declarations
template <ComputeStage>
class ADDerivativeParsedMaterial;

declareADValidParams(ADDerivativeParsedMaterial);

/**
 * Parsed material using ExprTk
 */
template <ComputeStage compute_stage>
class ADDerivativeParsedMaterial : public ADParsedMaterialBase<compute_stage>
{
public:
  ADDerivativeParsedMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// buffer for passing variable to exprtk
  std::vector<ADReal> _args_buffer;

  /// function value derivative
  std::vector<ADMaterialProperty(Real) *> _prop_dFdargs;

  ExprTkWrapper<ADReal> _exprtk;

  usingParsedMaterialBaseMembers;
};

#endif // ADDERIVATIVEPARSEDMATERIAL_H
