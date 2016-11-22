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

#ifndef QUASIPERIODICTESTKERNEL_H
#define QUASIPERIODICTESTKERNEL_H

#include "InterfaceKernel.h"

//Forward Declarations
class QuasiPeriodicTestKernel;

template<>
InputParameters validParams<QuasiPeriodicTestKernel>();

/**
 * DG kernel for interfacing a test problem between the same variable on quasiperiodic boundaries
 */
class QuasiPeriodicTestKernel : public InterfaceKernel
{
public:
  QuasiPeriodicTestKernel(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type);
  virtual Real computeQpJacobian(Moose::DGJacobianType type);

  Real _D;
  Real _D_neighbor;
  Real _offset;
};

#endif
