#ifndef EXTREMEMATERIALINDEXAUX_H
#define EXTREMEMATERIALINDEXAUX_H

#include "AuxKernel.h"

class ExtremeMaterialIndexAux;

template<>
InputParameters validParams<ExtremeMaterialIndexAux>();

/**
 * Return the index of the lowest material property in a vector at qp 0
 */
class ExtremeMaterialIndexAux : public AuxKernel
{
public:
  ExtremeMaterialIndexAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// What type of extreme value we are going to compute
  enum ExtremeType
  {
    MAX,
    MIN
  } _type;

  /// material properties to analyze
  std::vector<MaterialPropertyName> _prop_names;

  /// material properties to analyze
  std::vector<const MaterialProperty<Real> *> _props;

  /// index cache
  unsigned int _index;
};

#endif // EXTREMEMATERIALINDEXAUX_H
