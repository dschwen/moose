//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RigidSideset.h"
#include "RotationMatrix.h"

#include "libmesh/parallel_algebra.h"

registerMooseObject("TensorMechanicsApp", RigidSideset);

InputParameters
RigidSideset::validParams()
{
  InputParameters params = SideUserObject::validParams();
  params.addClassDescription("Compute the average translation and rotation of a sideset");
  params.addRequiredParam<Point>("origin", "Axis origin");
  params.addClassDescription("Compute the field of angular rotations of points around an axis "
                             "defined by an origin point and a direction vector");
  params.addRequiredCoupledVar("displacements", "The displacements");
  return params;
}

RigidSideset::RigidSideset(const InputParameters & parameters)
  : SideUserObject(parameters),
    _origin(getParam<Point>("origin")),
    _disp(coupledValues("displacements"))
{
}

void
RigidSideset::initialize()
{
  _qpdata.clear();
  _weight = 0;
  _shift.zero();
}

void
RigidSideset::execute()
{
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
  {
    // displacement vector
    RealVectorValue delta;
    for (unsigned int i = 0; i < _disp.size(); ++i)
      delta(i) = (*_disp[i])[qp];

    // weight
    auto weight = _JxW[qp] * _coord[qp];

    // average translations
    _weight += weight;
    _shift += weight * delta;

    // save off points for the rotation stage
    _qpdata.emplace_back(_q_point[qp], delta, weight);
  }
}

void
RigidSideset::finalize()
{
  // average translation
  gatherSum(_weight);
  gatherSum(_shift);
  _shift /= _weight;

  // average axis of rotation
  RealVectorValue sumaxis;
  for (const auto & p : _qpdata)
  {
    // undisplaced vector
    auto undisplaced = p._q_point - _origin;

    // weight for this qp (skip if we're at the origin)
    Real weight = undisplaced.norm();
    if (weight < libMesh::TOLERANCE)
      continue;
    weight *= p._weight;

    // translation corrected displacement
    auto delta = p._delta - _shift;

    // axis candidate is the normalized cross product of undisplaced vector and delta
    auto axis = undisplaced.cross(delta);
    auto anorm = axis.norm();
    if (anorm < libMesh::TOLERANCE)
      continue;
    axis /= anorm;
    sumaxis += weight * axis;
  }

  // parallel communicate to obtain averages
  gatherSum(sumaxis);
  auto anorm = sumaxis.norm();
  if (anorm < libMesh::TOLERANCE)
  {
    // no rotation
    _rotation.zero();
    _rotation(0, 0) = 1.0;
    _rotation(1, 1) = 1.0;
    _rotation(2, 2) = 1.0;
    return;
  }
  sumaxis /= anorm;

  // average rotation angle
  Real sumweight = 0.0;
  Real sumangle = 0.0;
  for (const auto & p : _qpdata)
  {
    // translation corrected displacement
    auto delta = p._delta - _shift;

    // undisplaced and displaced locations relative to the origin.
    RealVectorValue dr1 = p._q_point - _origin;
    RealVectorValue dr2 = dr1 + delta;

    // subtract out of plane projections
    dr1 -= sumaxis * (sumaxis * dr1);
    dr2 -= sumaxis * (sumaxis * dr2);

    // weight for this qp (skip if we're at the origin)
    auto dr1norm = dr1.norm();
    if (dr1norm < libMesh::TOLERANCE)
      continue;
    auto weight = dr1norm * p._weight;

    // product of the lengths
    auto norms = dr1norm * dr2.norm();

    // angle between dr1 and dr2
    if (norms > libMesh::TOLERANCE)
    {
      sumangle +=
          weight * std::acos((dr1 * dr2) / norms) * ((dr1.cross(dr2) * sumaxis) > 0 ? 1.0 : -1.0);
      sumweight += weight;
    }
  }

  // parallel communicate to obtain averages
  gatherSum(sumweight);
  gatherSum(sumangle);
  sumangle /= sumweight;

  // build rotation matrix
  _rotation = RotationMatrix::rotAboutAxis(sumangle, sumaxis);
}

void
RigidSideset::threadJoin(const UserObject & y)
{
  const RigidSideset & uo = static_cast<const RigidSideset &>(y);
  _qpdata.insert(_qpdata.end(), uo._qpdata.begin(), uo._qpdata.end());
  _weight += uo._weight;
  _shift += uo._shift;
}
