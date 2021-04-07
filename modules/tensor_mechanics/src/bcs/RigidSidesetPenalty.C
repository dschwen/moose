//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RigidSidesetPenalty.h"
#include "RigidSideset.h"
#include "Function.h"
#include "MooseEnum.h"

registerMooseObject("TensorMechanicsApp", RigidSidesetPenalty);

InputParameters
RigidSidesetPenalty::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription(
      "Apply a force in the direction of the rigidly transformed reference configuration");
  params.addRequiredParam<UserObjectName>("rigid_sideset", "RigidSideset user object");
  params.addParam<FunctionName>("penalty", 1e3, "Penalty factor (unit dependent)");
  MooseEnum componentEnum("x y z");
  params.addParam<MooseEnum>("component", componentEnum, "Component this BC acts on");
  return params;
}

RigidSidesetPenalty::RigidSidesetPenalty(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _penalty(getFunction("penalty")),
    _component(getParam<MooseEnum>("component")),
    _rigid_sideset(getUserObject<RigidSideset>("rigid_sideset")),
    _origin(_rigid_sideset.getOrigin()),
    _rotation(_rigid_sideset.getRotation()),
    _shift(_rigid_sideset.getShift())
{
}

Real
RigidSidesetPenalty::computeQpResidual()
{
  // rotate and shift the undisplaced point to where the rigid sideset should be
  auto target_displacement =
      _rotation * (_q_point[_qp] - _origin) + _origin + _shift - _q_point[_qp];
  return _penalty.value(_t, _q_point[_qp]) * (_u[_qp] - target_displacement(_component)) *
         _test[_i][_qp];
}

Real
RigidSidesetPenalty::computeQpJacobian()
{
  return _penalty.value(_t, _q_point[_qp]) * _phi[_j][_qp] * _test[_i][_qp];
}
