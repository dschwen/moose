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

#ifndef QUASIPERIODICGRADIENTTESTKERNEL_H
#define QUASIPERIODICGRADIENTTESTKERNEL_H

#include "InterfaceKernel.h"

//Forward Declarations
class QuasiPeriodicGradientTestKernel;

template<>
InputParameters validParams<QuasiPeriodicGradientTestKernel>();

/**
 * DG kernel for interfacing a test problem between the same variable on quasiperiodic boundaries
 */
class QuasiPeriodicGradientTestKernel : public InterfaceKernel
{
public:
  QuasiPeriodicGradientTestKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type);
  virtual Real computeQpJacobian(Moose::DGJacobianType type);

  const unsigned int _component;
  const Real _penalty;
  const Real _D;
  const Real _D_neighbor;
};

#endif
