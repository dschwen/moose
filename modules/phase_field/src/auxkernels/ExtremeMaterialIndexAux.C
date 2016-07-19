#include "ExtremeMaterialIndexAux.h"

template<>
InputParameters validParams<ExtremeMaterialIndexAux>()
{
  InputParameters params = validParams<AuxKernel>();

  // Define the min/max enumeration
  MooseEnum type_options("max=0 min=1", "min");
  params.addParam<MooseEnum>("value_type", type_options, "Type of extreme value to return. 'max' returns the maximum value. 'min' returns the minimum value.");

  params.addParam<std::vector<MaterialPropertyName>>("props", "Vector of material property names out of which the extreme value index will be chosen.");
  return params;
}

ExtremeMaterialIndexAux::ExtremeMaterialIndexAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _type((ExtremeType)(int)parameters.get<MooseEnum>("value_type")),
    _prop_names(getParam<std::vector<MaterialPropertyName>>("props")),
    _props(_prop_names.size())
{
  unsigned int n = _prop_names.size();
  if (n < 1)
    mooseError("Provide at least one material property");

  for (unsigned int i = 0; i < n; ++i)
    _props[i] = &getMaterialProperty<Real>(_prop_names[i]);
}

Real
ExtremeMaterialIndexAux::computeValue()
{
  // perform calculation at QP 0 only, avoid averaging integer indices
  if (_qp > 0)
    return _index;

  _index = 0;
  for (unsigned int i = 1; i < _props.size(); ++i)
    if ((_type == MAX && (*_props[i])[0] > (*_props[_index])[0]) ||
        (_type == MIN && (*_props[i])[0] < (*_props[_index])[0]))
      _index = i;

  return _index;
}
