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

#ifndef SHAPEELEMENTUSEROBJECT_H
#define SHAPEELEMENTUSEROBJECT_H

#include "ElementUserObject.h"

//Forward Declarations
class ShapeElementUserObject;

template<>
InputParameters validParams<ShapeElementUserObject>();

/**
 * ElementUserObject class in which the _phi and _grad_phi shape function data
 * is available and correctly initialized on EXEC_NONLINEAR (the Jacobian calculation).
 * This enables the calculation of Jacobian matrix contributions inside a UO.
 *
 * \warning It is up to the user to ensure _fe_problem.currentlyComputingJacobian()
 *          returns true before utilizing the shape functions.
 */
class ShapeElementUserObject : public ElementUserObject
{
public:
  ShapeElementUserObject(const InputParameters & parameters);

  /// returns the set of variables a Jacobian has bee requested for
  const std::set<MooseVariable *> & jacobianMooseVariables() { return _jacobian_moose_variables; }

  /**
   * This function will be called with the shape functions for jvar initialized. It
   * can be used to compute Jacobian contributions of the UserObject.
   */
  virtual void executeJacobian(unsigned int /*jvar*/) {}

protected:
  /**
   * Returns the index for a coupled variable by name and requests the computation
   * of a Jacobian w.r.t. to this variable i.e. the call to executeJacobian() with
   * shapefunctions initialized for this variable.
   */
  virtual unsigned int coupled(const std::string & var_name, unsigned int comp = 0);

  /// shape function values
  const VariablePhiValue & _phi;

  /// shape function gradients
  const VariablePhiGradient & _grad_phi;

private:
  std::set<MooseVariable *> _jacobian_moose_variables;
};

#endif //SHAPEELEMENTUSEROBJECT_H
