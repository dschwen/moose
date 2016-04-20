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

#ifndef INTEGRATEDPERIODICBC_H
#define INTEGRATEDPERIODICBC_H

#include "IntegratedBC.h"

// Forward declarations
class IntegratedPeriodicBC;
class MooseVariable;

template<>
InputParameters validParams<IntegratedPeriodicBC>();

/**
 * Base class for deriving periodic boundary conditions of a integrated type
 */
class IntegratedPeriodicBC :
  public IntegratedBC
{
public:
  IntegratedPeriodicBC(const InputParameters & parameters);

  void meshChanged();
  void computeResidual();

protected:
  /// A pointer to the periodic boundary constraints object
  PeriodicBoundaries *_pbs;

  ///@{ periodic boundary pair
  BoundaryID _current_boundary_id;
  BoundaryName _opposite_boundary_name;
  BoundaryID _opposite_boundary_id;
  ///@}

  /// opposite elements
  std::map<Elem *,Elem *> _opposite_elem_map;
};

#endif /* INTEGRATEDPERIODICBC_H */
