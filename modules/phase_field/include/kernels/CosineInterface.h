#ifndef COSINEINTERFACE_H
#define COSINEINTERFACE_H

#include "Kernel.h"

class CosineInterface;

template<>
InputParameters validParams<CosineInterface>();

/**
 * Prescribe a cosine shaped interface
 */
class CosineInterface : public Kernel
{
public:
  CosineInterface(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  const Real _a;
};

#endif // COSINEINTERFACE_H
