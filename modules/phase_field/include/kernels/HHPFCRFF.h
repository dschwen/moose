#ifndef HHPFCRFF_H
#define HHPFCRFF_H

#include "KernelValue.h"

//Forward Declarations
class HHPFCRFF;

template<>
InputParameters validParams<HHPFCRFF>();

/**
 * TODO: This Kernel needs Documentation!!!
 */
class HHPFCRFF : public KernelValue
{
public:
  HHPFCRFF(const std::string & name, InputParameters parameters);

protected:
  virtual Real precomputeQpResidual();
  virtual Real precomputeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  Real _kernel_sign;
  const MaterialProperty<Real> & _prop;

  VariableValue * _coupled;
  unsigned int _coupled_var;
};

#endif //HHFPCRFF_H
