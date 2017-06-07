/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "EvaluateOnSphere.h"
#include <cmath>

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
  params.addParam<MooseEnum>("angles", deg_type, "Angular unit");
  return params;
}

EvaluateOnSphere::EvaluateOnSphere(const InputParameters & parameters)
  : Function(parameters),
    _function(getFunction("function")),
    _radius(getParam<Real>("radius")),
    _deg(getParam<MooseEnum>("angles") == "DEG")
{
}

Real
EvaluateOnSphere::value(Real t, const Point & p)
{
  Point sphere;

  // longitude
  sphere(0) = std::atan2(p(0), p(1));

  // lattitude
  Real z = p(2) / _radius;
  z = std::min(z, 1.0);
  z = std::max(-1.0, z);
  sphere(1) = std::asin(z);

  if (_deg)
    sphere *= 180.0 / libMesh::pi;

  return _function(t, sphere);
}
