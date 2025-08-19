//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/dof_map.h"
#include "libmesh/numeric_vector.h"

#include <unordered_map>
#include <vector>

/**
 * Helper to operate in the constrained subspace defined by libMesh hanging-node constraints.
 *
 * This class does not form dense P or P^T. It iterates libMesh::DofMap constraint rows.
 */
class ConstrainedSubspace
{
public:
  ConstrainedSubspace() = default;

  /** Build internal adjacency from a DofMap's constraint rows. */
  void rebuild(const libMesh::DofMap & dof_map);

  /** Return true if there are any constrained dofs. */
  bool hasConstraints() const { return !_slave_to_masters.empty(); }

  /** Prolong: u_full = P q_masters (rhs for inhomogeneous rows is ignored here). */
  void prolong(const libMesh::NumericVector<libMesh::Number> & q_m,
               libMesh::NumericVector<libMesh::Number> & u_full) const;

  /** Restrict: v_masters = P^T v_full. */
  void restrict(const libMesh::NumericVector<libMesh::Number> & v_full,
                libMesh::NumericVector<libMesh::Number> & v_m) const;

  /** Reduce lumped mass: m_tilde[j] = m_full[j] + sum_s (a_sj^2 * m_full[s]). */
  void reduce_lumped_mass(const libMesh::NumericVector<libMesh::Number> & m_full,
                          libMesh::NumericVector<libMesh::Number> & m_tilde) const;

private:
  // slave dof -> vector of (master dof, coefficient a_sj)
  std::unordered_map<libMesh::dof_id_type, std::vector<std::pair<libMesh::dof_id_type, double>>>
      _slave_to_masters;

  // No explicit master list; operate via constraint row maps
};
