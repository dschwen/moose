//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SWENumericalFluxHLLC.h"

registerMooseObject("ShallowWaterApp", SWENumericalFluxHLLC);

InputParameters
SWENumericalFluxHLLC::validParams()
{
  InputParameters params = SWENumericalFluxBase::validParams();
  params.addClassDescription(
      "HLLC numerical flux for 2D shallow-water equations with hydrostatic reconstruction.");
  return params;
}

SWENumericalFluxHLLC::SWENumericalFluxHLLC(const InputParameters & parameters)
  : SWENumericalFluxBase(parameters)
{
}

SWENumericalFluxHLLC::~SWENumericalFluxHLLC() {}

void
SWENumericalFluxHLLC::calcFlux(unsigned int /*iside*/,
                               dof_id_type /*ielem*/,
                               dof_id_type /*ineig*/,
                               const std::vector<Real> & uvec1,
                               const std::vector<Real> & uvec2,
                               const RealVectorValue & n,
                               std::vector<Real> & flux) const
{
  mooseAssert(uvec1.size() >= 3, "Expected at least 3 conservative variables on left");
  mooseAssert(uvec2.size() >= 3, "Expected at least 3 conservative variables on right");

  const Real nx = n(0);
  const Real ny = n(1);
  const Real tx = -ny;
  const Real ty = nx;

  // Optional hydrostatic reconstruction if bathymetry b is provided
  const bool has_b = (uvec1.size() >= 4 && uvec2.size() >= 4);
  Real hL = std::max(uvec1[0], 0.0);
  Real huL = (hL > _h_eps) ? uvec1[1] : 0.0;
  Real hvL = (hL > _h_eps) ? uvec1[2] : 0.0;
  Real hR = std::max(uvec2[0], 0.0);
  Real huR = (hR > _h_eps) ? uvec2[1] : 0.0;
  Real hvR = (hR > _h_eps) ? uvec2[2] : 0.0;

  if (has_b)
  {
    const Real bL = uvec1[3];
    const Real bR = uvec2[3];
    const Real etaL = hL + bL;
    const Real etaR = hR + bR;
    const Real bstar = std::max(bL, bR);
    const Real hLstar = std::max(0.0, etaL - bstar);
    const Real hRstar = std::max(0.0, etaR - bstar);
    const Real uLx = (hL > _h_eps) ? huL / hL : 0.0;
    const Real uLy = (hL > _h_eps) ? hvL / hL : 0.0;
    const Real uRx = (hR > _h_eps) ? huR / hR : 0.0;
    const Real uRy = (hR > _h_eps) ? hvR / hR : 0.0;
    hL = hLstar;
    hR = hRstar;
    huL = uLx * hLstar;
    hvL = uLy * hLstar;
    huR = uRx * hRstar;
    hvR = uRy * hRstar;
  }

  // Decompose into normal/tangential components
  const Real unL = (hL > _h_eps) ? (huL * nx + hvL * ny) / hL : 0.0;
  const Real utL = (hL > _h_eps) ? (huL * tx + hvL * ty) / hL : 0.0;
  const Real unR = (hR > _h_eps) ? (huR * nx + hvR * ny) / hR : 0.0;
  const Real utR = (hR > _h_eps) ? (huR * tx + hvR * ty) / hR : 0.0;
  const Real cL = std::sqrt(_g * std::max(hL, 0.0));
  const Real cR = std::sqrt(_g * std::max(hR, 0.0));

  // Wave speed estimates
  const Real SL = std::min(unL - cL, unR - cR);
  const Real SR = std::max(unL + cL, unR + cR);
  const Real denom = (SR * hR - SL * hL);
  Real SM = 0.0;
  if (std::abs(denom) > 1e-12)
    SM = (SR * hR * (unR - SR) - SL * hL * (unL - SL)) / denom;
  else
    SM = 0.5 * (unL + unR);

  // Star depths
  const Real hLstar = (SL <= SM) ? hL * (SL - unL) / (SL - SM) : hL;
  const Real hRstar = (SM <= SR) ? hR * (SR - unR) / (SR - SM) : hR;

  // Physical fluxes
  auto Fn = [&](Real h, Real hu, Real hv, Real un) {
    std::vector<Real> f(3, 0.0);
    f[0] = h * un;
    f[1] = hu * un + 0.5 * _g * h * h * nx;
    f[2] = hv * un + 0.5 * _g * h * h * ny;
    return f;
  };

  const auto UL = std::vector<Real>{hL, huL, hvL};
  const auto UR = std::vector<Real>{hR, huR, hvR};
  const auto FL = Fn(hL, huL, hvL, unL);
  const auto FR = Fn(hR, huR, hvR, unR);

  // Star state momenta from normal/tangential decomposition
  const Real huLstar = hLstar * (SM * nx + utL * tx);
  const Real hvLstar = hLstar * (SM * ny + utL * ty);
  const Real huRstar = hRstar * (SM * nx + utR * tx);
  const Real hvRstar = hRstar * (SM * ny + utR * ty);

  const auto ULstar = std::vector<Real>{hLstar, huLstar, hvLstar};
  const auto URstar = std::vector<Real>{hRstar, huRstar, hvRstar};

  // HLLC flux selection
  flux.resize(3);
  if (0.0 <= SL)
    flux = FL;
  else if (SL <= 0.0 && 0.0 <= SM)
  {
    for (unsigned int i = 0; i < 3; ++i)
      flux[i] = FL[i] + SL * (ULstar[i] - UL[i]);
  }
  else if (SM <= 0.0 && 0.0 <= SR)
  {
    for (unsigned int i = 0; i < 3; ++i)
      flux[i] = FR[i] + SR * (URstar[i] - UR[i]);
  }
  else
    flux = FR;
}

void
SWENumericalFluxHLLC::calcJacobian(unsigned int /*iside*/,
                                   dof_id_type /*ielem*/,
                                   dof_id_type /*ineig*/,
                                   const std::vector<Real> & uvec1,
                                   const std::vector<Real> & uvec2,
                                   const RealVectorValue & n,
                                   DenseMatrix<Real> & jac1,
                                   DenseMatrix<Real> & jac2) const
{
  // Provide approximate Jacobians based on physical flux + Rusanov smax*I
  jac1.resize(3, 3);
  jac2.resize(3, 3);
  jac1.zero();
  jac2.zero();

  const Real nx = n(0);
  const Real ny = n(1);

  auto fill_dF = [&](const std::vector<Real> & U, DenseMatrix<Real> & J) {
    const Real h = std::max(U[0], 0.0);
    const Real hu = (h > _h_eps) ? U[1] : 0.0;
    const Real hv = (h > _h_eps) ? U[2] : 0.0;
    const Real invh = (h > _h_eps) ? 1.0 / h : 0.0;
    const Real un = (h > _h_eps) ? (hu * nx + hv * ny) * invh : 0.0;
    J(0, 0) = 0.0;
    J(0, 1) = nx;
    J(0, 2) = ny;
    if (h > _h_eps)
    {
      const Real qn = hu * nx + hv * ny;
      const Real d_un_dh = -qn * invh * invh;
      const Real d_un_dhu = nx * invh;
      const Real d_un_dhv = ny * invh;
      J(1, 0) = hu * d_un_dh + _g * h * nx;
      J(1, 1) = un + hu * d_un_dhu;
      J(1, 2) = hu * d_un_dhv;
      J(2, 0) = hv * d_un_dh + _g * h * ny;
      J(2, 1) = hv * d_un_dhu;
      J(2, 2) = un + hv * d_un_dhv;
    }
  };

  fill_dF(uvec1, jac1);
  fill_dF(uvec2, jac2);

  // smax
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
  for (unsigned int i = 0; i < 3; ++i)
  {
    jac1(i, i) += 0.5 * smax;
    jac2(i, i) += 0.5 * smax;
  }
}

