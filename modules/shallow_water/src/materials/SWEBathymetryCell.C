//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SWEBathymetryCell.h"
#include "Function.h"

#include "libmesh/elem.h"
// #include "libmesh/typedefs.h"

registerMooseObject("ShallowWaterApp", SWEBathymetryCell);

InputParameters
SWEBathymetryCell::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Bathymetry b provided as cell-centered (piecewise-constant).");
  params.addRequiredParam<FunctionName>("function", "Bed elevation function b(x,y)");
  return params;
}

SWEBathymetryCell::SWEBathymetryCell(const InputParameters & parameters)
  : Material(parameters),
    _bed_fun(getFunction("function")),
    _b(declareProperty<Real>("b")),
    _cached_id(libMesh::invalid_uint),
    _b_cell(0.0)
{
}

void
SWEBathymetryCell::computeQpProperties()
{
  const dof_id_type id = _current_elem->id();
  if (_cached_id != id)
  {
    // Representative point for the cell value
    const Point xc = _current_elem->centroid();
    _b_cell = _bed_fun.value(_t, xc);
    _cached_id = id;
  }

  // Same property value for all qps within the element (volume or side)
  _b[_qp] = _b_cell;
}

