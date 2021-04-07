//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RigidSidesetAux.h"
#include "RigidSideset.h"

registerMooseObject("TensorMechanicsApp", RigidSidesetAux);

InputParameters
RigidSidesetAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription(
      "Compute the distance of a sideset to the rigidly transformed reference configuration");
  params.addRequiredParam<UserObjectName>("rigid_sideset", "RigidSideset user object");
  params.addRequiredCoupledVar("displacements", "The displacements");
  return params;
}

RigidSidesetAux::RigidSidesetAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _disp(coupledValues("displacements")),
    _rigid_sideset(getUserObject<RigidSideset>("rigid_sideset")),
    _origin(_rigid_sideset.getOrigin()),
    _rotation(_rigid_sideset.getRotation()),
    _shift(_rigid_sideset.getShift())
{
  // sanity checks
  if (getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh", "This AuxKernel must be run on the undisplaced mesh");
  if (!isNodal())
    paramError("variable", "This AuxKernel must operate on a nodal variable");
  if (_disp.size() > LIBMESH_DIM)
    paramError("displacements",
               "Too many displacement variables were specified. The max is LIBMESH_DIM, which is ",
               LIBMESH_DIM);
}

Real
RigidSidesetAux::computeValue()
{
  // displacement vector
  RealVectorValue delta;
  for (unsigned int i = 0; i < _disp.size(); ++i)
    delta(i) = (*_disp[i])[_qp];

  // rotate and shift the undisplaced point to where the rigid sideset should be
  auto target_displacement =
      _rotation * (*_current_node - _origin) + _origin + _shift - *_current_node;

  return (delta - target_displacement).norm();
}
