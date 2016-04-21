/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "IntegratedPeriodicBC.h"
#include "SubProblem.h"
#include "SystemBase.h"
#include "NonlinearSystem.h"
#include "MooseVariable.h"
#include "Assembly.h"
#include "MooseMesh.h"

// libMesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<IntegratedPeriodicBC>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<BoundaryName>("other_boundary", "The opposite side boundary ID from the mesh where this boundary condition applies");
  return params;
}

IntegratedPeriodicBC::IntegratedPeriodicBC(const InputParameters & parameters) :
    IntegratedBC(parameters)
{
  // Extract the blocks from the input
  _opposite_boundary_name = parameters.get<BoundaryName>("other_boundary");

  std::vector<BoundaryName> boundary_names = parameters.get<std::vector<BoundaryName> >("boundary");
  if (boundary_names.size() != 1)
    mooseError("Supply only one single boundary pair per IntegratedPeriodicBC");

  // Get the IDs from the supplied names
  _current_boundary_id = _mesh.getBoundaryID(boundary_names[0]);
  _opposite_boundary_id = _mesh.getBoundaryID(_opposite_boundary_name);

  rebuildOppositeElementMap();
}

void
IntegratedPeriodicBC::meshChanged()
{
  rebuildOppositeElementMap();
}

void
IntegratedPeriodicBC::rebuildOppositeElementMap()
{
  // loop over boundary elements
  MooseMesh::bnd_elem_iterator bnd_it = _mesh.bndElemsBegin();
  for (; bnd_it != _mesh.bndElemsEnd(); _bnd_it)
  {

  }
}

void
IntegratedPeriodicBC::computeResidual()
{
  _pbs = dynamic_cast<FEProblem *>(&_subproblem)->getNonlinearSystem().dofMap().get_periodic_boundaries();

  // initialize the element on the opposite side
  Elem * _opposite_elem = neighbor(_current_boundary_id, _current_elem, );

  IntegratedBC::computeResidual();
}
