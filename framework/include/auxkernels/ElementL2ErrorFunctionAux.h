/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef ELEMENTL2ERRORFUNCTIONAUX_H
#define ELEMENTL2ERRORFUNCTIONAUX_H

// MOOSE includes
#include "FunctionAux.h"

// Forward declarations
class ElementL2ErrorFunctionAux;

template<>
InputParameters validParams<ElementL2ErrorFunctionAux>();

/**
 * A class for computing the L2 error of the difference between an
 * exact solution (typically represented by a ParsedFunction) and the
 * specified solution variable.
 */
class ElementL2ErrorFunctionAux : public FunctionAux
{
public:
  /**
   * Class constructor
   * @param name Object name
   * @param parameters Object input parameters
   */
  ElementL2ErrorFunctionAux(const InputParameters & parameters);

  /**
   * Override the base class functionality to compute the element
   * integral rather than scaling by element volume.
   */
  virtual void compute();

protected:
  /**
   * Computes the error
   */
  virtual Real computeValue();

  /// A reference to the solution variable to compute the error from
  const VariableValue & _soln;
};

#endif // ELEMENTL2ERRORFUNCTIONAUX_H
