//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Material.h"
#include "SerialAccess.h"
#include "RankTwoTensorForward.h"
#include "RankFourTensorForward.h"

/**
 * PowerLawSoftening is a smeared crack softening model that
 * uses a power law equation to soften the tensile response.
 * It is for use with ComputeSmearedCrackingStress.
 */
class InterpolatedStatefulMaterial : public Material
{
public:
  static InputParameters validParams();

  InterpolatedStatefulMaterial(const InputParameters & parameters);

  virtual void computeQpProperties() override;

  template <typename T>
  void declareProperties();

  template <typename T>
  void computeValues();

protected:
  /// Old projected state
  const std::vector<const VariableValue *> _old_state;

  /// Older projected state
  const std::vector<const VariableValue *> _older_state;

  /// emitted property name
  const MaterialPropertyName _prop_name;

  /// property type
  const MooseEnum _prop_type;

  template <typename T, int I>
  struct DeclareProperty
  {
    static void apply(InterpolatedStatefulMaterial * self) { self->declareProperties<T>(); }
  };

  template <typename T, int I>
  struct ProcessProperty
  {
    static void apply(InterpolatedStatefulMaterial * self) { self->computeValues<T>(); }
  };

  /// Old interpolated property
  PropertyValue * _prop_old;

  /// Older interpolated properties
  PropertyValue * _prop_older;
};

template <typename T>
void
InterpolatedStatefulMaterial::declareProperties()
{
  if (_prop_type != typeid(T).name())
    return;
  _prop_old = &declareProperty<T>(_prop_name + "_interpolated_old");
  _prop_older = &declareProperty<T>(_prop_name + "_interpolated_older");
}

template <typename T>
void
InterpolatedStatefulMaterial::computeValues()
{
  auto * prop_old = dynamic_cast<MaterialProperty<T>*>(_prop_old);
  if (!prop_old)
    return;
  auto * prop_older = static_cast<MaterialProperty<T>*>(_prop_older);

  std::size_t index = 0;
  for (auto & v : Moose::serialAccess((*prop_old)[_qp]))
    v = (*_old_state[index++])[_qp];

  index = 0;
  for (auto & v : Moose::serialAccess((*prop_older)[_qp]))
    v = (*_older_state[index++])[_qp];
}