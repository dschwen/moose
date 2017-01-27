/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CHPFCRFF_H
#define CHPFCRFF_H

#include "Kernel.h"

//Forward Declarations
class CHPFCRFF;

template <>
InputParameters validParams<CHPFCRFF>();

/**
 * This kernel calculates the main portion of the Cahn-Hilliard residual for the
 * RFF form of the phase field crystal model
 */
class CHPFCRFF : public Kernel
{
public:
  CHPFCRFF(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

private:
  const MaterialProperty<Real> & _M;
  bool _has_MJac;
  const MaterialProperty<Real> * _DM;

  MooseEnum _log_approach;
  Real _tol;
  unsigned int _n_exp_terms;
  Real _a;
  Real _b;
  Real _c;

  unsigned int _num_L;
  std::vector<unsigned int> _vals_var;
  std::vector<const VariableGradient *> _grad_vals;
};

#endif //CHPFCRFF_H
