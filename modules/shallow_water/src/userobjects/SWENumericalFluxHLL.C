//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SWENumericalFluxHLL.h"

registerMooseObject("ShallowWaterApp", SWENumericalFluxHLL);

InputParameters
SWENumericalFluxHLL::validParams()
{
  InputParameters params = SWENumericalFluxBase::validParams();
  params.addClassDescription(
      "HLL/Rusanov-style numerical flux for 2D shallow-water equations (stub).");
  return params;
}

SWENumericalFluxHLL::SWENumericalFluxHLL(const InputParameters & parameters)
  : SWENumericalFluxBase(parameters)
{
}

SWENumericalFluxHLL::~SWENumericalFluxHLL() {}

void
SWENumericalFluxHLL::calcFlux(unsigned int /*iside*/,
                              dof_id_type /*ielem*/,
                              dof_id_type /*ineig*/,
                              const std::vector<Real> & uvec1,
                              const std::vector<Real> & uvec2,
                              const RealVectorValue & n,
                              std::vector<Real> & flux) const
{
  mooseAssert(uvec1.size() == 3, "Expected 3 conservative variables on left");
  mooseAssert(uvec2.size() == 3, "Expected 3 conservative variables on right");

  const Real nx = n(0);
  const Real ny = n(1);

  const Real hL = std::max(uvec1[0], 0.0);
  const Real huL = (hL > _h_eps) ? uvec1[1] : 0.0;
  const Real hvL = (hL > _h_eps) ? uvec1[2] : 0.0;
  const Real hR = std::max(uvec2[0], 0.0);
  const Real huR = (hR > _h_eps) ? uvec2[1] : 0.0;
  const Real hvR = (hR > _h_eps) ? uvec2[2] : 0.0;

  const Real unL = (hL > _h_eps) ? (huL * nx + hvL * ny) / hL : 0.0;
  const Real unR = (hR > _h_eps) ? (huR * nx + hvR * ny) / hR : 0.0;
  const Real cL = std::sqrt(_g * std::max(hL, 0.0));
  const Real cR = std::sqrt(_g * std::max(hR, 0.0));
  const Real smax = std::max(std::fabs(unL) + cL, std::fabs(unR) + cR);

  // physical flux projected on n
  auto Fn = [&](Real h, Real hu, Real hv, Real un) {
    std::vector<Real> f(3, 0.0);
    f[0] = h * un;
    f[1] = hu * un + 0.5 * _g * h * h * nx;
    f[2] = hv * un + 0.5 * _g * h * h * ny;
    return f;
  };

  const auto FL = Fn(hL, huL, hvL, unL);
  const auto FR = Fn(hR, huR, hvR, unR);

  flux.resize(3);
  for (unsigned int i = 0; i < 3; ++i)
    flux[i] = 0.5 * (FL[i] + FR[i]) - 0.5 * smax * ((i == 0 ? hR : (i == 1 ? huR : hvR)) -
                                                    (i == 0 ? hL : (i == 1 ? huL : hvL)));
}

void
SWENumericalFluxHLL::calcJacobian(unsigned int /*iside*/,
                                  dof_id_type /*ielem*/,
                                  dof_id_type /*ineig*/,
                                  const std::vector<Real> & libmesh_dbg_var(uvec1),
                                  const std::vector<Real> & libmesh_dbg_var(uvec2),
                                  const RealVectorValue & /*n*/,
                                  DenseMatrix<Real> & jac1,
                                  DenseMatrix<Real> & jac2) const
{
  mooseAssert(uvec1.size() == 3, "Expected 3 conservative variables on left");
  mooseAssert(uvec2.size() == 3, "Expected 3 conservative variables on right");
  jac1.resize(3, 3);
  jac2.resize(3, 3);
  // Placeholder: set to zero. Proper analytic Jacobians can be added later.
  jac1.zero();
  jac2.zero();
}

