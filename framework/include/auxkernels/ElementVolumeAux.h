#ifndef ELEMENTVOLUMEAUX_H
#define ELEMENTVOLUMEAUX_H

#include "AuxKernel.h"

class ElementVolumeAux;

template<>
InputParameters validParams<ElementVolumeAux>();

/**
 *
 */
class ElementVolumeAux : public AuxKernel
{
public:
  ElementVolumeAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

};

#endif // ELEMENTVOLUMEAUX_H
