//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "ProjectedStatefulMaterialStorageAction.h"
#include "RankFourTensorForward.h"
#include "RankTwoTensorForward.h"
#include "AddAuxVariableAction.h"
#include "AddVariableAction.h"
#include "Conversion.h"
#include "FEProblemBase.h"
#include "Factory.h"

#include "libmesh/string_to_enum.h"

// we need to add these variables after the material property types are known
registerMooseAction("MooseApp",
                    ProjectedStatefulMaterialStorageAction,
                    "setup_projected_properties");

registerMooseAction("MooseApp", ProjectedStatefulMaterialStorageAction, "add_aux_kernel");

InputParameters
ProjectedStatefulMaterialStorageAction::validParams()
{
  InputParameters params = Action::validParams();

  params.addClassDescription("Mark material properties for projected stateful storage.");

  params.addParam<MooseEnum>(
      "family",
      MooseEnum("LAGRANGE MONOMIAL", "LAGRANGE"),
      "Finite element variable family to project the material properties onto");
  params.addParam<MooseEnum>(
      "order",
      AddAuxVariableAction::getAuxVariableOrders(),
      "Finite element variable order to project the material properties onto");

  // block restrictions
  params += BlockRestrictable::validParams();

  params.addParam<std::vector<MaterialPropertyName>>(
      "projected_props", {}, "Material properties to project for stateful storage");
  return params;
}

ProjectedStatefulMaterialStorageAction::ProjectedStatefulMaterialStorageAction(
    const InputParameters & params)
  : Action(params),
    _prop_names(getParam<std::vector<MaterialPropertyName>>("projected_props")),
    _order(params.get<MooseEnum>("order")),
    _fe_type({Utility::string_to_enum<Order>(_order),
              Utility::string_to_enum<FEFamily>(params.get<MooseEnum>("family"))}),
    _var_type(AddVariableAction::variableType(_fe_type)),
    _pomps_prefix("_pomps_")
{
}

void
ProjectedStatefulMaterialStorageAction::addRelationshipManagers(
    Moose::RelationshipManagerType input_rm_type)
{
  auto params = _factory.getValidParams("ProjectedStatefulMaterialNodalPatchRecovery");
  addRelationshipManagers(input_rm_type, params);
}

void
ProjectedStatefulMaterialStorageAction::act()
{
  auto setup = [&](std::size_t size,
                   const MaterialPropertyName & prop_name,
                   const std::string & prop_type,
                   bool is_ad)
  {
    // create variables
    std::vector<VariableName> vars;
    for (const auto i : make_range(size))
      vars.push_back("_var_" + prop_name + '_' + Moose::stringify(i));

    if (_current_task == "setup_projected_properties")
    {
      // add material
      {
        auto params = _factory.getValidParams("InterpolatedStatefulMaterial");
        params.applySpecificParameters(parameters(), {"block"});
        params.set<std::vector<VariableName>>("old_state") = vars;
        params.set<MooseEnum>("prop_type") = prop_type;
        params.set<MaterialPropertyName>("prop_name") = prop_name;
        _problem->addMaterial("InterpolatedStatefulMaterial", "_mat_" + prop_name, params);
      }

      // add the AuxVars for storage
      for (const auto & var : vars)
      {
        auto params = _factory.getValidParams(_var_type);
        params.applyParameters(parameters());
        params.set<std::vector<OutputName>>("outputs") = {"none"};
        _problem->addAuxVariable(_var_type, var, params);
      }

      // use nodal patch recovery for lagrange
      if (_fe_type.family == LAGRANGE)
      {
        // nodal variables require patch recovery (add user object)
        const auto & type_name = is_ad ? "ADProjectedStatefulMaterialNodalPatchRecovery"
                                       : "ProjectedStatefulMaterialNodalPatchRecovery";
        auto params = _factory.getValidParams(type_name);
        params.applySpecificParameters(parameters(), {"block"});
        // params.set<std::vector<unsigned int>>("component") = idx; // todo patch recover all
        // components at the same time
        params.set<MaterialPropertyName>("property") = prop_name;
        params.set<MooseEnum>("patch_polynomial_order") = _order;
        params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
        params.set<bool>("force_preaux") = true;
        _problem->addUserObject(type_name, uo_name, params);
      }
    }

    if (_current_task == "add_aux_kernel")
    {
      // use nodal patch recovery for lagrange
      if (_fe_type.family == LAGRANGE)
      {
        // nodal variables require patch recovery (add aux kernel)
        for (const auto & var : vars)
        {
          auto params = _factory.getValidParams("NodalPatchRecoveryAux");
          params.applySpecificParameters(parameters(), {"block"});
          params.set<AuxVariableName>("variable") = var;
          // params.set<UserObjectName>("nodal_patch_recovery_uo") = uo_name;
          params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
          _problem->addAuxKernel("NodalPatchRecoveryAux", "_aux_" + prop_name, params);
        }
      }
      else
      {
        // elemental variables
        {
          const auto & type_name =
              is_ad ? "ADProjectedStatefulMaterialAux" : "ProjectedStatefulMaterialAux";
          for (const auto i : index_range(vars))
          {
            auto params = _factory.getValidParams(type_name);
            params.applySpecificParameters(parameters(), {"block"});
            params.set<AuxVariableName>("variable") = vars[i];
            params.set<unsigned int>("component") = i;
            params.set<MaterialPropertyName>("property") = prop_name;
            params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_TIMESTEP_END};
            _problem->addAuxKernel(type_name, "_aux_" + prop_name, params);
          }
        }
      }
    }
  };

  const auto & data = _problem->getMaterialData(Moose::BLOCK_MATERIAL_DATA);
  for (const auto & prop_name : _prop_names)
  {
    // loop over all supported property types
    Moose::typeLoop<ProcessProperty>(SupportedTypes{}, &data, prop_name, setup);
  }
}
