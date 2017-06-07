//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MINMAXFUNCTIONIC_H
#define MINMAXFUNCTIONIC_H

#include "FunctionIC.h"

class MinMaxFunctionIC;

template <>
InputParameters validParams<MinMaxFunctionIC>();

/**
 * Assign the maximum/minimum of either given coupled variable or a given function
 */
class MinMaxFunctionIC : public FunctionIC
{
public:
  MinMaxFunctionIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  /// coupled variable to compare to the function
  const VariableValue & _v;

  /// compute maximum
  const bool _max;
};

#endif // MINMAXFUNCTIONIC_H
