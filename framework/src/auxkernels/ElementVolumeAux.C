#include "ElementVolumeAux.h"

template<>
InputParameters validParams<ElementVolumeAux>()
{
  InputParameters params = validParams<AuxKernel>();
  return params;
}

ElementVolumeAux::ElementVolumeAux(const InputParameters & parameters) : AuxKernel(parameters)
{
  if (isNodal())
    mooseError("Must be elemental");
}

Real
ElementVolumeAux::computeValue()
{
  return _current_elem->volume();
}
