/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/


#ifndef RVEBC_H
#define RVEBC_H

#include "IntegratedBC.h"


//Forward Declarations
class RVEBC;
class RankTwoTensor;
class RankFourTensor;

template<>
InputParameters validParams<RVEBC>();

class RVEBC : public IntegratedBC
{
public:
  /**
   * Factory constructor, takes parameters so that all derived classes can be built using the same
   * constructor.
   */
  RVEBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  unsigned int _component;
  const MaterialProperty<RankTwoTensor> & _stress;
  const MaterialProperty<RankFourTensor> & _Jacobian_mult;
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;
};

#endif //RVEBC_H
