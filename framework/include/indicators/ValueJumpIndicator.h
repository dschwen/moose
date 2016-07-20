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

#ifndef VALUEJUMPINDICATOR_H
#define VALUEJUMPINDICATOR_H

#include "JumpIndicator.h"

class ValueJumpIndicator;

template<>
InputParameters validParams<ValueJumpIndicator>();

class ValueJumpIndicator :
  public JumpIndicator
{
public:
  ValueJumpIndicator(const InputParameters & parameters);
  virtual ~ValueJumpIndicator(){};

protected:

  virtual Real computeQpIntegral();
};

#endif /* VALUEJUMPINDICATOR_H */
