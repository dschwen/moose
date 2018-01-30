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

#ifndef SCALARCOUPLEDTEST_H
#define SCALARCOUPLEDTEST_H

#include "ODEKernel.h"

class ScalarCoupledTest;

template <>
InputParameters validParams<ScalarCoupledTest>();

/**
 *
 */
class ScalarCoupledTest : public ODEKernel, public Coupleable
{
public:
  ScalarCoupledTest(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  const bool _up_coupling;
  const bool _down_coupling;
  const VariableValue & _c_up;
  const VariableValue & _c_down;
};

#endif // SCALARCOUPLEDTEST_H
