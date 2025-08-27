//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SWEWallBoundaryFlux.h"

registerMooseObject("ShallowWaterApp", SWEWallBoundaryFlux);

InputParameters
SWEWallBoundaryFlux::validParams()
{
  InputParameters params = BoundaryFluxBase::validParams();
  params.addClassDescription("Solid wall boundary flux for SWE: zero normal velocity.");
  params.addParam<Real>("gravity", 9.81, "Gravitational acceleration g");
  params.addParam<Real>("dry_depth", 1e-6, "Depth threshold for dry state");
  return params;
}

SWEWallBoundaryFlux::SWEWallBoundaryFlux(const InputParameters & parameters)
  : BoundaryFluxBase(parameters), _g(getParam<Real>("gravity")), _h_eps(getParam<Real>("dry_depth"))
{
}

SWEWallBoundaryFlux::~SWEWallBoundaryFlux() {}

void
SWEWallBoundaryFlux::calcFlux(unsigned int /*iside*/,
                              dof_id_type /*ielem*/,
                              const std::vector<Real> & U,
                              const RealVectorValue & n,
                              std::vector<Real> & flux) const
{
  mooseAssert(U.size() >= 3, "Expected at least 3 conservative variables");
  const Real nx = n(0), ny = n(1);
  const Real h = std::max(U[0], 0.0);

  flux.resize(3);
  // Zero normal flow: mass flux 0; momentum flux is hydrostatic pressure
  flux[0] = 0.0;
  flux[1] = 0.5 * _g * h * h * nx;
  flux[2] = 0.5 * _g * h * h * ny;
}

void
SWEWallBoundaryFlux::calcJacobian(unsigned int /*iside*/,
                                  dof_id_type /*ielem*/,
                                  const std::vector<Real> & U,
                                  const RealVectorValue & n,
                                  DenseMatrix<Real> & J) const
{
  mooseAssert(U.size() >= 3, "Expected at least 3 conservative variables");
  const Real nx = n(0), ny = n(1);
  const Real h = std::max(U[0], 0.0);
  J.resize(3, 3);
  J.zero();
  // d/dh of pressure terms
  J(1, 0) = _g * h * nx;
  J(2, 0) = _g * h * ny;
}

