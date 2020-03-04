//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TransientNormalizing.h"
#include "libmesh/utility.h"

registerMooseObject("MooseApp", TransientNormalizing);

InputParameters
TransientNormalizing::validParams()
{
  InputParameters params = Transient::validParams();
  params.addClassDescription(
      "Executioner for time varying simulations with variable vector noermalization.");
  params.addParam<std::vector<NonlinearVariableName>>(
      "normalize_vector", "List of variable to treat as a vector to normalize.");
  return params;
}

TransientNormalizing::TransientNormalizing(const InputParameters & parameters)
  : Transient(parameters),
    _vector_var_names(getParam<std::vector<NonlinearVariableName>>("normalize_vector")),
    _vector_var(_vector_var_names.size())
{
}

void
TransientNormalizing::init()
{
  for (unsigned int i = 0; i < _vector_var.size(); ++i)
    _vector_var[i] = &_problem.getVariable(0, _vector_var_names[i]);
}

void
TransientNormalizing::postStep()
{
  SystemBase & sys = _problem.systemBaseNonlinear();

  // normalize variables
  std::vector<std::vector<dof_id_type>> dofs(_vector_var.size());
  for (unsigned int i = 0; i < _vector_var.size(); ++i)
  {
    if (_vector_var[i]->count() != 1)
      mooseError("Variable ", _vector_var_names[i], " is not a scalar field variable!");

    std::set<dof_id_type> dof_set;
    sys.system().local_dof_indices(_vector_var[i]->number(), dof_set);
    std::copy(dof_set.begin(), dof_set.end(), dofs[i].begin());
  }

  for (unsigned int i = 1; i < _vector_var.size(); ++i)
    if (dofs[i].size() != dofs[0].size())
      mooseError("inconsistent dof size set");

  for (unsigned int j = 0; j < dofs[0].size(); ++j)
  {
    Real norm = 0;
    for (unsigned int i = 0; i < _vector_var.size(); ++i)
      norm += Utility::pow<2>(sys.solution()(dofs[i][j]));

    norm = std::sqrt(norm);

    for (unsigned int i = 0; i < _vector_var.size(); ++i)
      sys.solution().set(dofs[i][j], sys.solution()(dofs[i][j]) / norm);
  }

  sys.solution().close();
  sys.update();

  // call parent
  Transient::postStep();
}
