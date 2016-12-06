/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "DiscreteNucleationBase.h"
#include "DiscreteNucleationMap.h"

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<DiscreteNucleationBase>()
{
  InputParameters params = validParams<DerivativeFunctionMaterialBase>();
  params.addClassDescription("Free energy contribution for nucleating discrete particles");
  params.addRequiredParam<UserObjectName>("map", "DiscreteNucleationMap user object");
  params.addParam<Real>("penalty", 20.0, "Penalty factor for enforcing the target concentrations");
  MooseEnum penalty_mode("MATCH MIN MAX", "MATCH");
  params.addParam<MooseEnum>("penalty_mode", penalty_mode, "Match the target concentration or take it as a minimum or maximum concentration target");
  return params;
}

DiscreteNucleationBase::DiscreteNucleationBase(const InputParameters & params) :
    DerivativeFunctionMaterialBase(params),
    _penalty(getParam<Real>("penalty")),
    _penalty_mode(getParam<MooseEnum>("penalty_mode")),
    _map(getUserObject<DiscreteNucleationMap>("map"))
{
  // this does not work with legacy UO initialization
  if (_fe_problem.legacyUoInitialization())
    mooseError("DiscreteNucleation needs to be run with legacy UO initialization disabled. Set Problem/use_legacy_uo_initialization=false");
}
