//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExprTkWrapper.h"
#include "DualReal.h"

#include "metaphysicl/numberarray.h"
#include "metaphysicl/dualnumber.h"

#include "exprtk_dualnumber.h"

template <typename T>
ExprTkWrapper<T>::ExprTkWrapper()
  : _symbol_table(libmesh_make_unique<SymbolTable>()),
    _expression(libmesh_make_unique<Expression>()),
    _parser(libmesh_make_unique<Parser>())
{
  _symbol_table->add_constants();
  _expression->register_symbol_table(*_symbol_table);
}

template <typename T>
void
ExprTkWrapper<T>::addVariable(const std::string & name, T & var)
{
  _symbol_table->add_variable(name, var);
}

template <typename T>
bool
ExprTkWrapper<T>::parse(const std::string & expression_string)
{
  return _parser->compile(expression_string, *_expression);
}

// explicit instantiations
template class ExprTkWrapper<Real>;
template class ExprTkWrapper<DualReal>;
