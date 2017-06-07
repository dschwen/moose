/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "LoadXYZIC.h"
#include <fstream>

template<>
InputParameters validParams<LoadXYZIC>()
{
  InputParameters params = validParams<InitialCondition>();
  params.addClassDescription("Load an XYZ file");
  params.addRequiredParam<FileName>("file", "Filename of the XYZ file");
  params.addRequiredParam<unsigned int>("numx", "Number of x grid points");
  params.addRequiredParam<unsigned int>("numy", "Number of y grid points");
  params.addParam<Real>("xmin", 0.0, "Minimum x value");
  params.addParam<Real>("ymin", 0.0, "Minimum y value");
  params.addRequiredParam<Real>("xmax", "Maximum x value");
  params.addRequiredParam<Real>("ymax", "Maximum y value");
  return params;
}

LoadXYZIC::LoadXYZIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _numx(getParam<unsigned int>("numx")),
    _numy(getParam<unsigned int>("numy")),
    _xmin(getParam<Real>("xmin")),
    _xmax(getParam<Real>("xmax")),
    _ymin(getParam<Real>("ymin")),
    _ymax(getParam<Real>("ymax")),
    _zdata(_numx * _numy)
{
  FileName file = getParam<FileName>("file");
  std::ifstream stream_in(file);
  if (!stream_in)
    mooseError("Can't open file: ", file);

  Real x, y, z;
  while (stream_in >> x >> y >> z)
  {
    if (x >= _xmin && y >= _ymin && x < _xmax && y < _ymax)
    {
      const unsigned int ax = _numx * (x - _xmin) / (_xmax - _xmin);
      const unsigned int ay = _numy * (y - _ymin) / (_ymax - _ymin);

      _zdata[ax + _numx * ay] = z;
    }
  }
}

Real
LoadXYZIC::value(const Point & p)
{
  const unsigned int ax = _numx * (p(0) - _xmin) / (_xmax - _xmin);
  const unsigned int ay = _numy * (p(1) - _ymin) / (_ymax - _ymin);

  return _zdata[ax + _numx * ay];
}
