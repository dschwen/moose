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

  /// look up property type MooseNum values
  template <typename T>
  struct TypeName;

protected:
  /**
   * Add the material object to obtain the interpolated old state (for use with
   * InterpolatedStatefulMaterialPropertyInterface)
   */
  void addMaterial(const std::string & prop_type,
                   const std::string & prop_name,
                   std::vector<VariableName> & vars);

  template <typename T, int I>
  struct ProcessProperty
  {
    template <typename L>
    static void apply(const MaterialData * data, const MaterialPropertyName & prop_name, L lambda)
    {
      if (data->haveProperty<T>(prop_name))
        lambda(Moose::SerialAccess<T>::size(), prop_name, TypeName<T>::value, false);
      if (data->haveADProperty<T>(prop_name))
        lambda(Moose::SerialAccess<T>::size(), prop_name, TypeName<T>::value, true);
    }
  };

  const std::vector<MaterialPropertyName> & _prop_names;

  const MooseEnum _order;
  FEType _fe_type;
  const std::string _var_type;
  const std::string _pomps_prefix;
};
