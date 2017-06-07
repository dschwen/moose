//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef GRADSQUARE_H
#define GRADSQUARE_H

#include "Kernel.h"

class GradSquare;

template <>
InputParameters validParams<GradSquare>();

/**
 * Term representing the square of a (coupled) variable gradient
 */
class GradSquare : public Kernel
{
public:
  GradSquare(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
  virtual Real computeQpOffDiagJacobian(unsigned int jvar) override;

  /// is the kernel used in a coupled form?
  const bool _is_coupled;

  const VariableGradient & _grad_v;
  unsigned int _v_var;
};

#endif // GRADSQUARE_H
