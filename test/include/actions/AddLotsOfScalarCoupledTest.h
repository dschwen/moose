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

#ifndef ADDLOTSOFSCALARCOUPLEDTEST_H
#define ADDLOTSOFSCALARCOUPLEDTEST_H

#include "AddVariableAction.h"

class AddLotsOfScalarCoupledTest;

template <>
InputParameters validParams<AddLotsOfScalarCoupledTest>();

class AddLotsOfScalarCoupledTest : public AddVariableAction
{
public:
  AddLotsOfScalarCoupledTest(const InputParameters & params);

  virtual void act();
};

#endif // ADDLOTSOFSCALARCOUPLEDTEST_H
