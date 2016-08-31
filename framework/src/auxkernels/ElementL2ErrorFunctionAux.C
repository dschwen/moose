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

// MOOSE includes
#include "ElementL2ErrorFunctionAux.h"

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<ElementL2ErrorFunctionAux>()
{
  InputParameters params = validParams<FunctionAux>();
  params.addRequiredCoupledVar("solution_variable", "The variable to compare the function against");
  return params;
}

ElementL2ErrorFunctionAux::ElementL2ErrorFunctionAux(const InputParameters & parameters) :
    FunctionAux(parameters),
    _soln(coupledValue("solution_variable"))
{
}

void
ElementL2ErrorFunctionAux::compute()
{
  precalculateValue();

  if (isNodal())
    mooseError("ElementL2ErrorFunctionAux only makes sense as an Elemental AuxVariable.");

  // Sum up the squared-error values by calling computeValue(), then return the sqrt.
  Real summed_value = 0;
  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
    summed_value += _JxW[_qp] * _coord[_qp] * computeValue();

  _var.setNodalValue(std::sqrt(summed_value));
}

Real
ElementL2ErrorFunctionAux::computeValue()
{
  // Get exact solution from base class computeValue() function.
  Real exact = FunctionAux::computeValue();

  // Difference between exact and computed solution at the current qp.
  Real diff = exact - _soln[_qp];

  // Return difference-squared.  The compute() function sums these and
  // takes the square root.
  return diff*diff;
}
