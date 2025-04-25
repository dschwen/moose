//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include <ATen/ops/zeros_like.h>
#ifdef NEML2_ENABLED

#include "libmesh/id_types.h"
#include <ATen/ops/max.h>
#include <ATen/ops/from_blob.h>

#include "libmesh/petsc_vector.h"

// MOOSE includes
#include "MooseError.h"
#include "NEML2Residual.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseApp.h"

registerMooseObject("MooseApp", NEML2Residual);

InputParameters
NEML2Residual::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addParam<std::vector<NonlinearVariableName>>("variables", "Coupled variables");
  return params;
}

NEML2Residual::NEML2Residual(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _variable_names(getParam<std::vector<NonlinearVariableName>>("variables")),
    _solution(dynamic_cast<const PetscVector<Real> *>(_sys.currentSolution())),
    _residual(dynamic_cast<PetscVector<Real> *>(&_sys.getVector("nontime")))
{
  // check if the solution vector is of a supported type
  if (!_solution)
    mooseError("Only solution vectors of type PetscVector are currently supported");

  if (sizeof(Real) != 8 && sizeof(sizeof(Real)) != 4)
    mooseError("Unsupported Real number size");

  // coupled variables must be nonlinear scalar fields
  for (const auto & variable_name : _variable_names)
  {
    const auto * var = &_fe_problem.getVariable(
        0, variable_name, Moose::VarKindType::VAR_SOLVER, Moose::VarFieldType::VAR_FIELD_STANDARD);

    // check block restriction
    if (var->blockRestricted())
      paramError("variables", "cannot be block restricted.");

    _variables.push_back(var);
  }
}

void
NEML2Residual::initialSetup()
{
  meshChanged();
}

void
NEML2Residual::meshChanged()
{
  int64_t nnode = 0;
  int64_t nelem = 0;
  int64_t nvar = _variables.size();
  std::vector<dof_id_type> var_dof_indices;

  _connectivity.clear();
  // iterate over the entire domain
  for (const auto & elem : *_fe_problem.mesh().getActiveLocalElementRange())
  {
    // get all dof indices for the coupled variables on this element
    if (nnode == 0)
      nnode = elem->n_nodes();
    else if (nnode != elem->n_nodes())
      // best check which type though
      mooseError("Must have one single element type in your domain.");

    // prepare DOF indices
    for (const auto i : index_range(_variables))
    {
      _variables[i]->getDofIndices(elem, var_dof_indices);

      // sanity checks
      if (_variables[i]->scalingFactor() != 1)
        mooseError("Scaling factors other than unity are not yet supported");

      for (const auto j : var_dof_indices)
        _connectivity.push_back(_solution->map_global_to_local_index(j));
    }
    nelem++;
  }

  const auto dsize = sizeof(libMesh::dof_id_type);
  if (dsize != 8 && dsize != 4)
    mooseError("Unsupported dof_id_type");
  _connectivity_tensor = at::from_blob(_connectivity.data(), {nelem, nvar, nnode}, torch::kInt64)
                             //  dsize == 8 ? torch::kInt64 : torch::kInt32)
                             .to(_app.getLibtorchDevice());
  _solution_size = torch::max(_connectivity_tensor).cpu().item<int64_t>();
}

void
NEML2Residual::execute()
{
  // grab solution vector (pinky promise not to write to it!)
  _solution_tensor = at::from_blob(const_cast<Real *>(_solution->get_array_read()),
                                   {static_cast<int64_t>(_solution_size)})
                         .to(_app.getLibtorchDevice());

  // run NEML2 stuff
  _residual_tensor = torch::zeros_like(_solution_tensor); // <- dummy

  // copy back residual
  torch::Tensor moose_residual_tensor =
      torch::from_blob(_residual->get_array(),
                       {static_cast<int64_t>(_solution_size)},
                       sizeof(Real) == 8 ? torch::kFloat64 : torch::kFloat32);
  moose_residual_tensor += _residual_tensor.cpu();

  // close solution and residual vector access
  const_cast<PetscVector<Real> *>(_solution)->restore_array();
  _residual->restore_array();
}

#endif
