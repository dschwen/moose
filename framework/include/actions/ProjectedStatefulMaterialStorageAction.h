//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Action.h"
#include "Conversion.h"
#include "SerialAccess.h"
#include "RankTwoTensorForward.h"
#include "RankFourTensorForward.h"
#include "libmesh/fe_type.h"

/**
 * Set up AuxKernels and AuxVariables for projected material property storage (PoMPS).
 */
class ProjectedStatefulMaterialStorageAction : public Action
{
public:
  static InputParameters validParams();

  ProjectedStatefulMaterialStorageAction(const InputParameters & parameters);

  virtual void act() override;

  using Action::addRelationshipManagers;
  virtual void addRelationshipManagers(Moose::RelationshipManagerType input_rm_type) override;

  /// List of supported types
  typedef Moose::TypeList<Real, RealVectorValue, RankTwoTensor, RankFourTensor> SupportedTypes;

  /// get an enum with all supported types
  static MooseEnum getTypeEnum();

protected:
  template <typename T, int I>
  struct ProcessProperty
  {
    template <typename L>
    static void apply(const MaterialData * data, const MaterialPropertyName & prop_name, L lambda)
    {
      if (data->haveProperty<T>(prop_name))
        lambda(Moose::SerialAccess<T>::size(), prop_name, typeid(T).name(), false);
      if (data->haveADProperty<T>(prop_name))
        lambda(Moose::SerialAccess<T>::size(), prop_name, typeid(T).name(), true);
    }
  };

  template <typename... Ts>
  static MooseEnum getTypeEnum(typename Moose::TypeList<Ts...>);

  const std::vector<MaterialPropertyName> & _prop_names;

  const MooseEnum _order;
  FEType _fe_type;
  const std::string _var_type;
  const std::string _pomps_prefix;
};

template <typename... Ts>
MooseEnum
ProjectedStatefulMaterialStorageAction::getTypeEnum(Moose::TypeList<Ts...>)
{
  return MooseEnum(Moose::stringify(std::vector<std::string>{typeid(Ts).name()...}, " "));
}
