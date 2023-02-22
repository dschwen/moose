//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralVariablePostprocessor.h"

/**
 * Test user object that computes a value only in finalize, and depends on the values
 * computed by another user object of the same type. Correctly resolving this dependency
 * requires multiple loops over the mesh and tests the execution_order_group system.
 */
class ExecutionGroupTestPostprocessor : public ElementIntegralVariablePostprocessor
{
public:
  static InputParameters validParams();

  ExecutionGroupTestPostprocessor(const InputParameters & parameters);

  virtual void initialize() override { _final_integral = 0.0; }
  virtual void finalize() override;
  Real getData() const { return _final_integral; }

protected:
  virtual Real computeQpIntegral() override;

  const ExecutionGroupTestPostprocessor * _chained_object;
  Real _final_integral;
};
