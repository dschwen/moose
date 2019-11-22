//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADFParser.h"

ADFParser::ADFParser(const ADFParser & cpy)
  : FunctionParserAD(cpy),
    _real_function(cpy._real_function),
    _dual_function(cpy._dual_function),
    _epsilon(1e-12)
{
}

Real
ADFParser::Eval(const Real * vars)
{
  return (*_real_function)(vars, pImmed, _epsilon);
}

DualReal
ADFParser::Eval(const DualReal * Vars)
{
  return (*_dual_function)(vars, pImmed, _epsilon);
}
