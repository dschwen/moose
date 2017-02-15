/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "OrderParameterInterfaceMarker.h"
#include "MooseVariable.h"
#include "Assembly.h"

// libMesh includes
#include "libmesh/quadrature.h"

template <>
InputParameters
validParams<OrderParameterInterfaceMarker>()
{
  InputParameters params = validParams<Marker>();
  params.addClassDescription("Adapt mesh to a target value change across an element. ");
  params.addCoupledVar("interface_vars", "Interface variables to base adaptivity on");
  params.addParam<std::vector<Real>>(
      "value_change",
      std::vector<Real>({1.0}),
      "Value change across an interface for each interface variable");
  params.addRangeCheckedParam<Real>("elems_across_interface",
                                    3.0,
                                    "elems_across_interface > 0",
                                    "target number of elements across an interface");
  return params;
}

OrderParameterInterfaceMarker::OrderParameterInterfaceMarker(const InputParameters & parameters)
  : Marker(parameters),
    Coupleable(this, true),
    _qrule(_assembly.qRule()),
    _n_vars(coupledComponents("interface_vars")),
    _var_nodal(_n_vars),
    _vals(_n_vars),
    _value_change(getParam<std::vector<Real>>("value_change")),
    _elems_across_interface(getParam<Real>("elems_across_interface"))
{
  if (_value_change.size() == 1)
    _value_change.assign(_n_vars, _value_change[0]);

  if (_value_change.size() != _n_vars)
    mooseError("Supply a value_change value for each interface_var");

  for (auto i = decltype(_n_vars)(0); i < _n_vars; ++i)
  {
    if (!getVar("interface_vars", i)->isNodal())
      mooseError("Variable needs to be nodal");

    _vals[i] = &coupledValue("interface_vars", i);
    _value_change[i] /= _elems_across_interface;
  }
}

Marker::MarkerValue
OrderParameterInterfaceMarker::computeElementMarker()
{
  // this will stay coarsen if none of the variables either require refinement or
  // or set the state to do nothing
  MarkerValue marker_value = COARSEN;

  for (auto i = decltype(_n_vars)(0); i < _n_vars; ++i)
  {
    mooseAssert(_vals.size() > 0, "Empty MooseVariable encountered");

    Real min = (*_vals[i])[0];
    Real max = min;
    for (unsigned int qp = 1; qp < _current_elem->n_nodes(); ++qp)
    {
      if ((*_vals[i])[qp] > max)
        max = (*_vals[i])[qp];
      if ((*_vals[i])[qp] < min)
        min = (*_vals[i])[qp];
    }

    std::cout << (*_vals[i])[0] << ", " << (*_vals[i])[1] << "  ";

    // if we are above the targeted change we need to refine
    if (max - min > _value_change[i])
      return REFINE;

    // just above half the targeted change means do nothing (coarsening would
    // require refinement in the next step)
    if (max - min > _value_change[i] / 2.0)
      return DO_NOTHING;
  }

  return marker_value;
}
