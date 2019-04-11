//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef EXPRTKWRAPPER_H
#define EXPRTKWRAPPER_H

#include "MooseTypes.h"

// forward declare main exprtk namespace classes
namespace exprtk
{
template <typename T>
class symbol_table;
template <typename T>
class expression;
template <typename T>
class parser;
};

/**
 * ExprTkWrapper is a thin wrapper class that is explicitly instantiated to all
 * number / dual number types used in MOOSE. This avoids having to include the
 * 1.3Mb large exprtk.h file of the header only ExprTk library directly in MOOSE
 * objects (which would have a substantial impact on compilation time).
 */
template <typename T>
class ExprTkWrapper
{
public:
  ExprTkWrapper();

  /// delete the dynamically allocated wrapped exprtk objects
  ~ExprTkWrapper();

  ///@{ templated ExprTk typedefs
  typedef exprtk::symbol_table<T> SymbolTable;
  typedef exprtk::expression<T> Expression;
  typedef exprtk::parser<T> Parser;
  ///@}

  /// evaluate the wrapped ExprTk expression
  T value();

  /// register a variable reference
  void addVariable(const std::string & name, T & var);

  /// parse a given expression
  bool parse(const std::string & expression_string);

protected:
  /**@{
   * Pointers to the wrapped exprtk objects. std::unique_ptr does not work here
   * unfortunately because it requires the type templates to be completely known,
   * which is exactly what we are trying to avoid here.
   */
  SymbolTable * _symbol_table;
  Expression * _expression;
  Parser * _parser;
  ///@}
};

#endif // EXPRTKWRAPPER_H
