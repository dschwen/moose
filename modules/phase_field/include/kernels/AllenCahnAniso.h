/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ALLENCAHNANISO_H
#define ALLENCAHNANISO_H

#include "AllenCahnBase.h"

/**
 * Residual of the Allen-Cahn
 * equation with a tensor (anisotropic) mobility.
 */
class AllenCahnAniso : public AllenCahnBase<RealTensorValue>
{
public:
  AllenCahnAniso(const InputParameters & parameters);
};

template <>
InputParameters validParams<AllenCahnAniso>();

#endif // ALLENCAHNANISO_H
