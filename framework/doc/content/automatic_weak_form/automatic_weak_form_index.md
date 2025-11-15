# Automatic Weak Form Generation Documentation

This directory contains comprehensive documentation for the automatic weak form generation capabilities in MOOSE.

## Overview

- [Automatic Weak Form Documentation](automatic_weak_form_documentation.md) - Main documentation covering features, usage, and implementation details
- [MOOSE Integration Plan](moose_integration_plan.md) - Plan for integrating automatic weak form into MOOSE framework

## Core Features

### Variable Splitting
- [Variable Splitting Documentation](variable_splitting_documentation.md) - Comprehensive guide to automatic variable splitting for higher-order PDEs

### Variational Derivative Operator
- [var_diff() Usage Guide](var_diff_usage.md) - How to use the `var_diff()` operator in energy expressions
- [var_diff() Implementation Summary](var_diff_implementation_summary.md) - Technical implementation details of the variational derivative operator

### Boundary Conditions
- [Boundary Term Extraction](boundary_term_extraction.md) - Automatic extraction and implementation of natural boundary conditions from energy functionals

## Verbose Output and Debugging

- [Verbose Output Summary](verbose_output_summary.md) - Quick reference for using verbose output features
- [Verbose Output Status](verbose_output_status.md) - Detailed analysis of verbose output implementation
- [Verbose Output Example](verbose_output_example.md) - Practical examples of verbose output usage

## Design Documents

- [Strong Form Design](moose_automatic_weak_form_strong_form_design.md) - Design document for strong form equation support

## Development and Improvements

- [Expression Simplifier Improvements](simplifier_improvements.md) - Planned improvements to the expression simplification system
- [Tolerance Improvements](tolerance_improvements.md) - Documentation of tolerance handling improvements

## Quick Start

For a quick introduction to using automatic weak form generation, start with:

1. [Automatic Weak Form Documentation](automatic_weak_form_documentation.md) - Read the main documentation
2. [var_diff() Usage Guide](var_diff_usage.md) - Learn how to specify energy functionals
3. [Verbose Output Summary](verbose_output_summary.md) - Enable debugging output for your simulations

## Key Concepts

### Energy Functionals

Energy functionals are mathematical expressions representing the total energy of a system. The automatic weak form system computes variational derivatives to derive the weak form equations automatically.

### Variational Derivatives

The `var_diff()` operator computes variational derivatives δF/δu of energy functionals F[u]. This produces the Euler-Lagrange equations that govern the system.

### Variable Splitting

For PDEs with derivatives higher than the finite element order, the system automatically introduces auxiliary variables and constraint equations to reformulate the problem as a mixed system of lower-order equations.

### Natural Boundary Conditions

Boundary conditions arising from the integration-by-parts process are automatically extracted and implemented as `VariationalBC` objects.

## Implementation Architecture

The system consists of several key components:

- **AutomaticWeakFormAction** - Main action class orchestrating the weak form generation
- **StringExpressionParser** - Parses mathematical expressions into an AST
- **WeakFormGenerator** - Computes variational derivatives and generates weak form terms
- **VariableSplittingAnalyzer** - Analyzes expressions and generates split variables
- **ExpressionTransformer** - Transforms expressions to use split variables
- **VariationalKernelBase** - Base kernel implementing the weak form residuals

See the [Automatic Weak Form Documentation](automatic_weak_form_documentation.md) for detailed architecture information.

## Contributing

When adding new documentation, please:

1. Use lowercase filenames with underscores (e.g., `feature_name.md`)
2. Add a link to this index page
3. Include clear cross-references to related documentation
4. Provide examples where appropriate

## Support

For questions or issues:

- Check the [Verbose Output Summary](verbose_output_summary.md) for debugging tips
- Review the [var_diff() Usage Guide](var_diff_usage.md) for syntax examples
- Consult the main [MOOSE documentation](https://mooseframework.inl.gov)
