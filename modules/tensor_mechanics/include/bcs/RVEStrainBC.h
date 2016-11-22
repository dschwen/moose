/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef RVESTRAINBC_H
#define RVESTRAINBC_H

#include "IntegratedBC.h"


//Forward Declarations
class RVEStrainBC;

template<>
InputParameters validParams<RVEStrainBC>();

/**
 * Implements a simple constant Dashpot BC where grad(u)=value on the boundary.
 * Uses the term produced from integrating the diffusion operator by parts.
 */
class RVEStrainBC : public IntegratedBC
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  RVEStrainBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  unsigned int _component;
};

#endif //RVESTRAINBC_H
