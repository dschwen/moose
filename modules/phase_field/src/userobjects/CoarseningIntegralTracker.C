//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoarseningIntegralTracker.h"
#include "MooseVariable.h"
#include "MooseMesh.h"

// libmesh includes
#include "libmesh/quadrature.h"

registerMooseObject("PhaseFieldApp", CoarseningIntegralTracker);

template <>
InputParameters
validParams<CoarseningIntegralTracker>()
{
  InputParameters params = validParams<ElementUserObject>();
  params.addClassDescription("Track the pre-coarsening integral of elements.");
  params.addRequiredCoupledVar("v", "variable to correct");
  return params;
}

CoarseningIntegralTracker::CoarseningIntegralTracker(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _mesh(_fe_problem.mesh()),
    _v(coupledValue("v")),
    _v_var(getVar("v", 0)),
    _pre_adaptivity_ran(false)
{
  _fe_problem.requestCacheMeshChanged();
}

void
CoarseningIntegralTracker::initialize()
{
  // clear the stored child element integrals and set flag to indicate new data was collected
  _pre_adaptivity_integral.clear();
  _pre_adaptivity_ran = true;

  // clear the corrective source term for the current timestep
  _corrective_source.clear();
}

void
CoarseningIntegralTracker::execute()
{
  // only do this if this element has a parent
  const Elem * parent = _current_elem->parent();
  if (!parent)
    return;

  // calculate integral over this element
  Real sum = 0.0;
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
    sum += _JxW[qp] * _coord[qp] * _v[qp];

  // put integral contribution into the multimap, indexed by parent id
  _pre_adaptivity_integral.insert(std::make_pair(parent->id(), sum));
}

void
CoarseningIntegralTracker::meshChanged()
{
  // if the userobject was not scheduled to run due to lack of dependencies we need to bail out
  if (!_pre_adaptivity_ran)
    return;

  // loop over elements that just have been coarsened (currently not threaded)
  for (const auto & parent : *_mesh.coarsenedElementRange())
  {
    // compute sum of child integrals
    auto child_range = _pre_adaptivity_integral.equal_range(parent->id());
    Real child_sum = 0.0;
    for (auto it = child_range.first; it != child_range.second; ++it)
      child_sum += it->second;

    // compute coarsened parent integral
    _fe_problem.prepare(parent, 0);
    if (_current_elem_volume <= 0.0)
      mooseError("negative volume");
    _v_var->computeElemValues();

    Real parent_sum = 0.0;
    for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
      parent_sum += _JxW[qp] * _coord[qp] * _v[qp];

    // store corrective term
    _corrective_source.insert(
        std::make_pair(parent, (child_sum - parent_sum) / _current_elem_volume));
    // _corrective_source.insert(std::make_pair(parent, child_sum));
  }

  _pre_adaptivity_ran = false;
}

void
CoarseningIntegralTracker::threadJoin(const UserObject & y)
{
  const CoarseningIntegralTracker & uo = static_cast<const CoarseningIntegralTracker &>(y);
  _pre_adaptivity_integral.insert(uo._pre_adaptivity_integral.begin(),
                                  uo._pre_adaptivity_integral.end());
}

void
CoarseningIntegralTracker::finalize()
{
  std::vector<std::pair<dof_id_type, Real>> vecdata(_pre_adaptivity_integral.begin(),
                                                    _pre_adaptivity_integral.end());
  _communicator.allgather(vecdata, false);
  _pre_adaptivity_integral.clear();
  _pre_adaptivity_integral.insert(vecdata.begin(), vecdata.end());
}

Real
CoarseningIntegralTracker::sourceValue(const Elem * elem) const
{
  auto val_it = _corrective_source.find(elem);
  if (val_it == _corrective_source.end())
    return 0.0;
  else
    return val_it->second;
}
