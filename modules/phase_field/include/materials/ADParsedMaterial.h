//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADPARSEDMATERIAL_H
#define ADPARSEDMATERIAL_H

#include "ADParsedMaterialBase.h"

// Forward Declarations
template <ComputeStage>
class ADParsedMaterial;

declareADValidParams(ADParsedMaterial);

/**
 * Parsed material using ExprTk
 */
template <ComputeStage compute_stage>
class ADParsedMaterial : public ADParsedMaterialBase<compute_stage>
{
public:
  ADParsedMaterial(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// buffer for passing variable to exprtk
  std::vector<ADReal> _args_buffer;

  /// Expression parser backend object
  ExprTkWrapper<ADReal> _exprtk;

  usingParsedMaterialBaseMembers;
};

#endif // ADPARSEDMATERIAL_H
