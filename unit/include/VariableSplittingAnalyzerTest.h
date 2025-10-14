//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "gtest/gtest.h"
#include "MooseTypes.h"

// Forward declarations
namespace moose
{
namespace automatic_weak_form
{
class VariableSplittingAnalyzer;
class StringExpressionParser;
}
}

/**
 * Test fixture for VariableSplittingAnalyzer tests
 */
class VariableSplittingAnalyzerTest : public ::testing::Test
{
protected:
  void SetUp() override;

  std::unique_ptr<moose::automatic_weak_form::VariableSplittingAnalyzer> analyzer;
  std::unique_ptr<moose::automatic_weak_form::StringExpressionParser> parser;

  // Helper methods
  void expectSplitVariable(const std::map<std::string, moose::automatic_weak_form::SplitVariable> & split_vars,
                           const std::string & name,
                           const std::string & original_var,
                           unsigned int derivative_order);

  void expectNoSplitVariable(const std::map<std::string, moose::automatic_weak_form::SplitVariable> & split_vars,
                             const std::string & name);

  bool hasVariable(const std::map<std::string, unsigned int> & orders,
                   const std::string & var_name,
                   unsigned int expected_order);
};