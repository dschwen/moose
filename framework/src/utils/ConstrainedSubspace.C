//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstrainedSubspace.h"

#include "libmesh/dof_map.h"
#include "libmesh/system.h"

using namespace libMesh;

void ConstrainedSubspace::rebuild(const DofMap & dof_map)
{
  _slave_to_masters.clear();

  // Iterate constraint rows
  for (auto it = dof_map.constraint_rows_begin(); it != dof_map.constraint_rows_end(); ++it)
  {
    const dof_id_type slave = it->first;
    const auto & row = it->second; // DofConstraintRow

    std::vector<std::pair<dof_id_type, double>> masters;
    masters.reserve(row.size());
    for (const auto & kv : row)
    {
      const dof_id_type master = kv.first;
      const double a_sj = kv.second;
      masters.emplace_back(master, a_sj);
    }

    _slave_to_masters.emplace(slave, std::move(masters));
  }

  // No explicit master list needed; we operate directly on constraint rows
}

void ConstrainedSubspace::prolong(const NumericVector<Number> & q_m,
                                  NumericVector<Number> & u_full) const
{
  // Start with a copy (masters already present, slaves will be overwritten)
  u_full = q_m;

  // Fill slaves as linear combinations of masters
  for (const auto & it : _slave_to_masters)
  {
    const dof_id_type s = it.first;
    Number val = 0.0;
    for (const auto & pair : it.second)
      val += pair.second * q_m(pair.first);
    u_full.set(s, val);
  }
  u_full.close();
}

void ConstrainedSubspace::restrict(const NumericVector<Number> & v_full,
                                   NumericVector<Number> & v_m) const
{
  // Start with a copy (masters and slaves)
  v_m = v_full;

  // Accumulate P^T contribution: for each slave s, add a_sj * v_full[s] to master j
  for (const auto & it : _slave_to_masters)
  {
    const dof_id_type s = it.first;
    const Number vs = v_full(s);
    if (vs == 0.0)
      continue;
    for (const auto & pair : it.second)
      v_m.add(pair.first, pair.second * vs);
  }
  v_m.close();
}

void ConstrainedSubspace::reduce_lumped_mass(const NumericVector<Number> & m_full,
                                             NumericVector<Number> & m_tilde) const
{
  // Start with full-space diagonal
  m_tilde = m_full;

  // Add slave contributions a_sj^2 * m_s to each connected master j
  for (const auto & it : _slave_to_masters)
  {
    const dof_id_type s = it.first;
    const Number ms = m_full(s);
    if (ms == 0.0)
      continue;
    for (const auto & pair : it.second)
    {
      const dof_id_type j = pair.first;
      const double a = pair.second;
      m_tilde.add(j, a * a * ms);
    }
  }
  m_tilde.close();
}
