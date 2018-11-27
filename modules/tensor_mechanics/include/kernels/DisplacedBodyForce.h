//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DISPLACEDBODYFORCE_H
#define DISPLACEDBODYFORCE_H

#include "ALEKernel.h"

class DisplacedBodyForce;

template <>
InputParameters validParams<DisplacedBodyForce>();

/**
 * This kernel implements the Laplacian operator:
 * $\nabla u \cdot \nabla \phi_i$
 */
class DisplacedBodyForce : public ALEKernel
{
public:
  DisplacedBodyForce(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;

  /// Scale factor
  const Real & _scale;

  /// Optional function value
  Function & _function;

  /// Optional Postprocessor value
  const PostprocessorValue & _postprocessor;
};

#endif /* DISPLACEDBODYFORCE_H */
