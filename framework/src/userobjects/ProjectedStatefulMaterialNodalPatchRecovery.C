//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ProjectedStatefulMaterialNodalPatchRecovery.h"
#include "ProjectedStatefulMaterialStorageAction.h"
#include "ElementUserObject.h"
#include "MaterialBase.h"
#include "Assembly.h"

registerMooseObject("MooseApp", ProjectedStatefulMaterialNodalPatchRecovery);

InputParameters
ProjectedStatefulMaterialNodalPatchRecovery::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  return params;
}

ProjectedStatefulMaterialNodalPatchRecovery::ProjectedStatefulMaterialNodalPatchRecovery(
    const InputParameters & parameters)
  : NodalPatchRecoveryMaterialProperty(parameters),
    _current_subdomain_id(_assembly.currentSubdomainID())
{
  const auto & data = _problem->getMaterialData(Moose::BLOCK_MATERIAL_DATA);
  Moose::typeLoop<FetchProperty>(ProjectedStatefulMaterialStorageAction::SupportedTypes{}, this);
}

void
ProjectedStatefulMaterialNodalPatchRecovery::initialSetup()
{
  // get all material classes that provide properties for this object
  _required_materials = buildRequiredMaterials();
}

Real
ProjectedStatefulMaterialNodalPatchRecovery::execute()
{

  // auto setup = []() {};
  // // Moose::typeLoop<ProcessProperty>(ProjectedStatefulMaterialStorageAction::SupportedTypes{}, );
  return 0; //_prop[_qp];
}

Real
ProjectedStatefulMaterialNodalPatchRecovery::nodalPatchRecovery(
    const Point & x, const std::vector<dof_id_type> & elem_ids) const
{
  // Before we go, check if we have enough sample points for solving the least square fitting
  if (_q_point.size() * elem_ids.size() < _q)
    mooseError("There are not enough sample points to recover the nodal value, try reducing the "
               "polynomial order or using a higher-order quadrature scheme.");

  // Assemble the least squares problem over the patch
  RealEigenMatrix A = RealEigenMatrix::Zero(_q, _q);
  RealEigenVector b = RealEigenVector::Zero(_q);
  for (auto elem_id : elem_ids)
  {
    A += libmesh_map_find(_Ae, elem_id);
    b += libmesh_map_find(_be, elem_id);
  }

  // Solve the least squares fitting
  RealEigenVector coef = A.completeOrthogonalDecomposition().solve(b);

  // Compute the fitted nodal value
  RealEigenVector p = evaluateBasisFunctions(x);
  return p.dot(coef);
}

RealEigenVector
ProjectedStatefulMaterialNodalPatchRecovery::evaluateBasisFunctions(const Point & q_point) const
{
  RealEigenVector p(_q);
  Real polynomial;
  for (unsigned int r = 0; r < _multi_index.size(); r++)
  {
    polynomial = 1.0;
    mooseAssert(_multi_index[r].size() == _mesh.dimension(), "Wrong multi-index size.");
    for (unsigned int c = 0; c < _multi_index[r].size(); c++)
      for (unsigned int p = 0; p < _multi_index[r][c]; p++)
        polynomial *= q_point(c);
    p(r) = polynomial;
  }
  return p;
}

void
ProjectedStatefulMaterialNodalPatchRecovery::initialize()
{
  _abs.clear();
}

void
ProjectedStatefulMaterialNodalPatchRecovery::execute()
{
  if (_t_step == 0)
    for (const auto & mat : _required_materials[_current_subdomain_id])
      mat->initStatefulProperties(_qrule->size());

  std::vector<RealEigenVector> bs(_components);
  RealEigenMatrix Ae = RealEigenMatrix::Zero(_q, _q);

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    RealEigenVector p = evaluateBasisFunctions(_q_point[_qp]);
    Ae += p * p.transpose();

    std::size_t i = 0;
    auto compute = [&](const Real & v, std::size_t i) { bs[i] += v * p; };
    Moose::typeLoop<ProcessProperty>(
        ProjectedStatefulMaterialStorageAction::SupportedTypes{}, _prop, _qp, compute);
  }

  dof_id_type elem_id = _current_elem->id();
  _Ae[elem_id] = Ae;
  _be[elem_id] = be;
}

void
NodalPatchRecoveryBase::threadJoin(const UserObject & uo)
{
  const auto & npr = static_cast<const NodalPatchRecoveryBase &>(uo);
  _Ae.insert(npr._Ae.begin(), npr._Ae.end());
  _be.insert(npr._be.begin(), npr._be.end());
}

void
NodalPatchRecoveryBase::finalize()
{
  // When calling nodalPatchRecovery, we may need to know _Ae and _be on algebraically ghosted
  // elements. However, this userobject is only run on local elements, so we need to query those
  // information from other processors in this finalize() method.

  // Populate algebraically ghosted elements to query
  std::unordered_map<processor_id_type, std::vector<dof_id_type>> query_ids;
  const ConstElemRange evaluable_elem_range = _fe_problem.getEvaluableElementRange();
  for (auto elem : evaluable_elem_range)
    if (elem->processor_id() != processor_id())
      query_ids[elem->processor_id()].push_back(elem->id());

  typedef std::pair<RealEigenMatrix, RealEigenVector> AbPair;

  // Answer queries received from other processors
  auto gather_data = [this](const processor_id_type /*pid*/,
                            const std::vector<dof_id_type> & elem_ids,
                            std::vector<AbPair> & ab_pairs)
  {
    for (const auto i : index_range(elem_ids))
    {
      const auto elem_id = elem_ids[i];
      ab_pairs.emplace_back(libmesh_map_find(_Ae, elem_id), libmesh_map_find(_be, elem_id));
    }
  };

  // Gather answers received from other processors
  auto act_on_data = [this](const processor_id_type /*pid*/,
                            const std::vector<dof_id_type> & elem_ids,
                            const std::vector<AbPair> & ab_pairs)
  {
    for (const auto i : index_range(elem_ids))
    {
      const auto elem_id = elem_ids[i];
      const auto & [Ae, be] = ab_pairs[i];
      _Ae[elem_id] = Ae;
      _be[elem_id] = be;
    }
  };

  libMesh::Parallel::pull_parallel_vector_data<AbPair>(
      _communicator, query_ids, gather_data, act_on_data, 0);
}
