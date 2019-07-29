//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"
#include <vector>

class TupleProductSum;

template <>
InputParameters validParams<TupleProductSum>();

/**
 * Sum products of distinct subsets of a variable vector
 */
class TupleProductSum : public AuxKernel
{
public:
  TupleProductSum(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// construct subsets recursively
  void subset(std::vector<unsigned int> & partial, unsigned int index);

  /**
   * Order N of the tuples.
   * 1: a0 + a1 + a2
   * 2: a0*a1 + a1*a2 + a2*a0
   * 3: a0*a1*a2
   * etc.
   */
  const unsigned int _order;

  /// number of coupled variables
  const unsigned int _nvals;

  /// coupled variables
  std::vector<const VariableValue *> _vals;

  /// number of expected
  const unsigned int _nsubsets;

  /// subsets of size _order of _vals
  std::vector<unsigned int> _subsets;
};
