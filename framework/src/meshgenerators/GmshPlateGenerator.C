//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GmshPlateGenerator.h"

registerMooseObject("MooseApp", GmshPlateGenerator);

InputParameters
GmshPlateGenerator::validParams()
{
  InputParameters params = GmshGeneratorBase::validParams();
  params.set<int>("dim") = 2;
  params.addParam<Real>("spacing", 1e-2, "Mesh size");
  params.addParam<Real>("xmin", 0.0, "Lower X Coordinate of the generated mesh");
  params.addParam<Real>("xmax", 1.0, "Upper X Coordinate of the generated mesh");
  params.addParam<Real>("ymin", 0.0, "Lower Y Coordinate of the generated mesh");
  params.addParam<Real>("ymax", 1.0, "Upper Y Coordinate of the generated mesh");
  return params;
}

GmshPlateGenerator::GmshPlateGenerator(const InputParameters & parameters)
  : GmshGeneratorBase(parameters),
    _xmin(getParam<Real>("xmin")),
    _xmax(getParam<Real>("xmax")),
    _ymin(getParam<Real>("ymin")),
    _ymax(getParam<Real>("ymax"))
{
}

void
GmshPlateGenerator::generateGeometry()
{
  Real lc = getParam<Real>("spacing");
  gmsh::model::geo::addPlaneSurface({Moose::gmshAddRectangleLoop(_xmin, _xmax, _ymin, _ymax, lc)},
                                    1);
}
