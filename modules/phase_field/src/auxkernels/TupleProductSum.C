//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TupleProductSum.h"
#include "MathUtils.h"

#include "libmesh/utility.h"

registerMooseObject("PhaseFieldApp", TupleProductSum);

template <>
InputParameters
validParams<TupleProductSum>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addClassDescription("Sum products of distinct subsets of a variable vector.");
  params.addCoupledVar("v", "Vector of coupled variables to sum tuples of.");
  params.addRequiredParam<unsigned int>("k", "Subset size");
  return params;
}

TupleProductSum::TupleProductSum(const InputParameters & parameters)
  : AuxKernel(parameters),
    _order(getParam<unsigned int>("k")),
    _nvals(coupledComponents("v")),
    _vals(_nvals),
    _nsubsets(Utility::binomial(_nvals, _order))
{
  // Fetch variables
  for (std::size_t i = 0; i < _nvals; ++i)
    _vals[i] = &coupledValue("v", i);

  // Build subsets
  _subsets.reserve(_nsubsets * _order);
  std::vector<unsigned int> partial(_order);
  subset(partial, 0);
  mooseAssert(_subsets.size() == _nsubsets * _order, "unexpected _subsets size");
}

Real
TupleProductSum::computeValue()
{
  unsigned int k = 0;
  Real sum = 0.0;
  for (unsigned int i = 0; i < _nsubsets; ++i)
  {
    Real prod = 1.0;
    for (unsigned int j = 0; j < _order; ++j)
      prod *= (*_vals[_subsets[k++]])[_qp];
    sum += prod;
  }
  return sum;
}

void
TupleProductSum::subset(std::vector<unsigned int> & partial, unsigned int index)
{
  if (index == _order)
    _subsets.insert(_subsets.end(), partial.begin(), partial.end());
  else
  {
    const auto begin = index == 0 ? 0 : partial[index - 1] + 1;
    const auto end = _nvals - _order + index;
    for (unsigned int i = begin; i <= end; ++i)
    {
      partial[index] = i;
      subset(partial, index + 1);
    }
  }
}
