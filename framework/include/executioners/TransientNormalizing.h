//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Transient.h"

/**
 * Transient executioners that normalizes a set of solution variables corresponding
 * to a vector
 */
class TransientNormalizing : public Transient
{
public:
  static InputParameters validParams();

  TransientNormalizing(const InputParameters & parameters);

  void init() override;

  void postStep() override;

protected:
  std::vector<NonlinearVariableName> _vector_var_names;
  std::vector<MooseVariableFEBase *> _vector_var;
};
