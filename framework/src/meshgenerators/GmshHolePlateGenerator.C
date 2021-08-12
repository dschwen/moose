//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GmshHolePlateGenerator.h"
#include "libmesh/utility.h"

registerMooseObject("MooseApp", GmshHolePlateGenerator);

InputParameters
GmshHolePlateGenerator::validParams()
{
  InputParameters params = GmshPlateGenerator::validParams();
  params.addParam<std::vector<Real>>("x", "Circle hole x coordinates");
  params.addParam<std::vector<Real>>("y", "Circle hole y coordinates");
  params.addParam<std::vector<Real>>("r", "Circle hole radii");
  return params;
}

GmshHolePlateGenerator::GmshHolePlateGenerator(const InputParameters & parameters)
  : GmshPlateGenerator(parameters),
    _x(getParam<std::vector<Real>>("x")),
    _y(getParam<std::vector<Real>>("y")),
    _r(getParam<std::vector<Real>>("r")),
    _n_holes(_x.size())
{
  if (_n_holes != _y.size())
    paramError("y", "x and y vectors must be of identical length");
  if (_n_holes != _r.size())
    paramError("r", "Provide one radius per circle center");

  for (std::size_t i = 0; i < _n_holes; ++i)
    for (std::size_t j = 0; j < i; ++j)
      if (std::sqrt(Utility::pow<2>(_x[i] - _x[j]) + Utility::pow<2>(_y[i] - _y[j])) <
          _r[i] + _r[j])
        mooseError("Forbidden overlap between circle ", i, " and circle ", j);
}

void
GmshHolePlateGenerator::generateGeometry()
{
  Real lc = getParam<Real>("spacing");

  std::vector<int> tags{Moose::gmshAddRectangleLoop(_xmin, _xmax, _ymin, _ymax, lc)};
  for (std::size_t i = 0; i < _n_holes; ++i)
    tags.push_back(Moose::gmshAddCircleLoop(_x[i], _y[i], _r[i], lc));

  gmsh::model::geo::addPlaneSurface(tags, 1);
}
