//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterpolatedStatefulMaterial.h"
#include "ProjectedStatefulMaterialStorageAction.h"

registerMooseObject("MooseApp", InterpolatedStatefulMaterial);

InputParameters
InterpolatedStatefulMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Access old state from projected data.");
  params.addRequiredCoupledVar("old_state", "The AuxVars for the coupled components");
  params.addParam<MooseEnum>(
      "prop_type", ProjectedStatefulMaterialStorageAction::getTypeEnum(), "Property type");
  params.addRequiredParam<MaterialPropertyName>("prop_name", "Name to emit");
  return params;
}

InterpolatedStatefulMaterial::InterpolatedStatefulMaterial(const InputParameters & parameters)
  : Material(parameters),
    _old_state(coupledValuesOld("old_state")),
    _older_state(coupledValuesOld("old_state")),
    _prop_name(getParam<MaterialPropertyName>("prop_name")),
    _prop_type(getParam<MooseEnum>("prop_type"))
{
  Moose::typeLoop<DeclareProperty>(ProjectedStatefulMaterialStorageAction::SupportedTypes{}, this);
}

void
InterpolatedStatefulMaterial::computeQpProperties()
{
  Moose::typeLoop<ProcessProperty>(ProjectedStatefulMaterialStorageAction::SupportedTypes{}, this);
}
