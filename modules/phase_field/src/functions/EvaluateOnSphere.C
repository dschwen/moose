//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EvaluateOnSphere.h"
#include <cmath>

registerMooseObject("PhaseFieldApp", EvaluateOnSphere);

template <>
InputParameters
validParams<EvaluateOnSphere>()
{
  InputParameters params = validParams<Function>();
  params.addClassDescription("Converts the passed in cartesian coordinates to spherical "
                             "coordinates and passes them through to another function");
  params.addParam<FunctionName>("function", "Function that takes spherical coordinates");
  params.addParam<Real>("radius", "Radius of the sphere");
  MooseEnum angle_type("RAD DEG", "DEG");
  params.addParam<MooseEnum>("angles", angle_type, "Angular unit");
  params.addParam<Point>("angle_scale", Point(-1.0, 1.0, 1.0), "Angle scale factors");
  return params;
}

EvaluateOnSphere::EvaluateOnSphere(const InputParameters & parameters)
  : Function(parameters),
    _fe_problem(getParam<FEProblemBase *>("_fe_problem_base")),
    _tid(getParam<THREAD_ID>("_tid")),
    _function(_fe_problem->getFunction(getParam<FunctionName>("function"), _tid)),
    _radius(getParam<Real>("radius")),
    _angle_scale(getParam<Point>("angle_scale")),
    _deg(getParam<MooseEnum>("angles") == "DEG")
{
}

Real
EvaluateOnSphere::value(Real t, const Point & p)
{
  Point sphere;

  // longitude
  sphere(0) = std::atan2(p(0), p(1)) * _angle_scale(0);

  // latitude
  Real z = p(2) / _radius;
  z = std::min(z, 1.0);
  z = std::max(-1.0, z);
  sphere(1) = std::asin(z) * _angle_scale(1);

  if (_deg)
    sphere *= 180.0 / libMesh::pi;

  return _function.value(t, sphere);
}
