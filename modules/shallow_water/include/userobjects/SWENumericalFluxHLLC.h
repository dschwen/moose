//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SWENumericalFluxBase.h"

/**
 * HLLC numerical flux for 2D shallow-water equations with hydrostatic reconstruction.
 * Uses 1D HLLC in the face-normal direction; tangential velocity is advected unchanged.
 */
class SWENumericalFluxHLLC : public SWENumericalFluxBase
{
public:
  static InputParameters validParams();

  SWENumericalFluxHLLC(const InputParameters & parameters);
  virtual ~SWENumericalFluxHLLC();

  virtual void calcFlux(unsigned int iside,
                        dof_id_type ielem,
                        dof_id_type ineig,
                        const std::vector<Real> & uvec1,
                        const std::vector<Real> & uvec2,
                        const RealVectorValue & dwave,
                        std::vector<Real> & flux) const override;

  virtual void calcJacobian(unsigned int iside,
                            dof_id_type ielem,
                            dof_id_type ineig,
                            const std::vector<Real> & uvec1,
                            const std::vector<Real> & uvec2,
                            const RealVectorValue & dwave,
                            DenseMatrix<Real> & jac1,
                            DenseMatrix<Real> & jac2) const override;
};

