//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef LOADXYZIC_H
#define LOADXYZIC_H

#include "InitialCondition.h"

class LoadXYZIC;

template <>
InputParameters validParams<LoadXYZIC>();

/**
 * Loads an XYZ file
 */
class LoadXYZIC : public InitialCondition
{
public:
  LoadXYZIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  const unsigned int _numx, _numy;
  const Real _xmin, _xmax, _ymin, _ymax;

  std::vector<Real> _zdata;
};

#endif // LOADXYZIC_H
