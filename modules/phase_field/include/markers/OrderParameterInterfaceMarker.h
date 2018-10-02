//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ORDERPARAMETERINTERFACEMARKER_H
#define ORDERPARAMETERINTERFACEMARKER_H

#include "Marker.h"
#include "Coupleable.h"

class OrderParameterInterfaceMarker;

template <>
InputParameters validParams<OrderParameterInterfaceMarker>();

/**
 * Refine/corsen based on the difference between min and max nodal values.
 */
class OrderParameterInterfaceMarker : public Marker, public Coupleable
{
public:
  OrderParameterInterfaceMarker(const InputParameters & parameters);

protected:
  virtual MarkerValue computeElementMarker() override;

  /// The quadrature rule for the system
  QBase *& _qrule;

  /// number of tracked interface variables
  const unsigned int _n_vars;

  /// is interface variable nodal
  std::vector<bool> _var_nodal;

  /// interface variable values
  std::vector<const VariableValue *> _vals;

  /// upper limit on the value change across an element in the interface
  std::vector<Real> _value_change;

  /// number of elements required to resolve an interface
  const Real _elems_across_interface;
};

#endif // ORDERPARAMETERINTERFACEMARKER_H
