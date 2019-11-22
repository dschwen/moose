//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DualReal.h"
#include "libmesh/fparser_ad.hh"

class ADFParser : public FunctionParserAD
{
public:
  ADFParser(const ADFParser & cpy);

  Real Eval(const Real * Vars);
  DualReal Eval(const DualReal * Vars);

  void setRealFunction(void * func) { _real_function = func; }
  void setDualFunction(void * func) { _dual_function = func; }

protected:
  Real (*_real_function)(const Real *, const Real *, const Real);
  DualReal (*_dual_function)(const DualReal *, const Real *, const Real);
  const Real _epsilon;
};
