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

#ifndef RVEPERIODICSTRAIN_H
#define RVEPERIODICSTRAIN_H

#include "InterfaceKernel.h"

//Forward Declarations
class RVEPeriodicStrain;
class RankFourTensor;
class RankTwoTensor;

template<>
InputParameters validParams<RVEPeriodicStrain>();

/**
 * DG kernel for interfacing diffusion between two variables on adjacent blocks
 */
class RVEPeriodicStrain : public InterfaceKernel
{
public:
  RVEPeriodicStrain(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type);
  virtual Real computeQpJacobian(Moose::DGJacobianType type);

private:
  const unsigned int _component;
  const unsigned int _direction;
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;

  const unsigned int _disp_x_var;
  const unsigned int _disp_y_var;
  const unsigned int _disp_z_var;

  const VariableValue & _disp_x;
  const VariableValue & _disp_y;
  const VariableValue & _disp_z;

  const VariableGradient & _grad_disp_x;
  const VariableGradient & _grad_disp_y;
  const VariableGradient & _grad_disp_z;

  MooseVariable & _disp_x_var_test;
  MooseVariable & _disp_y_var_test;
  MooseVariable & _disp_z_var_test;

  const VariableGradient & _grad_disp_x_neighbor_value;
  const VariableGradient & _grad_disp_y_neighbor_value;
  const VariableGradient & _grad_disp_z_neighbor_value;

  const Real _penalty;
};

#endif
