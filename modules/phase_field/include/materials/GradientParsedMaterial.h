/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GRADIENTPARSEDMATERIAL_H
#define GRADIENTPARSEDMATERIAL_H

#include "ParsedMaterialHelper.h"
#include "FunctionMaterialBase.h"
#include "ParsedMaterialBase.h"

// Forward Declarations
class GradientParsedMaterial;

template<>
InputParameters validParams<GradientParsedMaterial>();

/**
 * FunctionMaterialBase child class to evaluate a parsed function and
 * compute its gradient.
 */
class GradientParsedMaterial : public ParsedMaterialHelper<FunctionMaterialBase>, public ParsedMaterialBase
{
public:
  GradientParsedMaterial(const std::string & name,
                         InputParameters parameters);

protected:
  virtual void computeProperties();

  /// The first derivatives of the free energy (function parser objects).
  std::vector<ADFunction *> _func_dF;

  /// argument gradients
  MaterialProperty<VariableGradient *> _grad_args;

  /// gradient material property
  MaterialProperty<RealGradient> & _grad_F;
};

#endif //GRADIENTPARSEDMATERIAL_H
