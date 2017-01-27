/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef PFCRFFENERGYDENSITY_H
#define PFCRFFENERGYDENSITY_H

#include "AuxKernel.h"
#include <sstream>

class PFCRFFEnergyDensity;

template<>
InputParameters validParams<PFCRFFEnergyDensity>();

class PFCRFFEnergyDensity : public AuxKernel
{
public:
   PFCRFFEnergyDensity( const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  unsigned int _order;
  std::vector<const VariableValue *> _vals;

  std::vector<Real> _coeffs;

  MooseEnum _log_approach;
  Real _tol;
};

#endif //PFCRFFENERGYDENSITY_H
