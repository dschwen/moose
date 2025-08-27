//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DGKernel.h"
#include "InternalSideFluxBase.h"

/**
 * DG side kernel that assembles finite-volume style fluxes for the
 * 2D shallow water equations using a numerical flux userobject.
 *
 * Variables are the conservative set [h, hu, hv]. The kernel is added
 * once per variable and will assemble residual and full 3x3 Jacobians.
 */
class SWEFVFluxDGKernel : public DGKernel
{
public:
  static InputParameters validParams();

  SWEFVFluxDGKernel(const InputParameters & parameters);
  virtual ~SWEFVFluxDGKernel();

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar) override;

  std::map<unsigned int, unsigned int> getIndexMapping() const;

  // element-side conservative values (no reconstruction in stub)
  const VariableValue & _h_elem;
  const VariableValue & _hu_elem;
  const VariableValue & _hv_elem;

  // neighbor-side conservative values
  const VariableValue & _h_neig;
  const VariableValue & _hu_neig;
  const VariableValue & _hv_neig;

  // numerical flux user object
  const InternalSideFluxBase & _numerical_flux;

  // variable indices for mapping to [h,hu,hv]
  const unsigned int _h_var;
  const unsigned int _hu_var;
  const unsigned int _hv_var;

  const std::map<unsigned int, unsigned int> _jmap;
  const unsigned int _equation_index;
};

