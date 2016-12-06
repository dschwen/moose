/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "DiscreteNucleationParsed.h"
#include "DiscreteNucleationMap.h"

// libmesh includes
#include "libmesh/quadrature.h"

template<>
InputParameters validParams<DiscreteNucleationParsed>()
{
  InputParameters params = validParams<DiscreteNucleationBase>();
  params.addClassDescription("Free energy contribution for nucleating discrete particles based on an indirect order parameter (such as the concentration in a Grand Potential model)");
  params.addRequiredCoupledVar("args", "List of variables the target order parameter property depends on");
  params.addRequiredParam<Real>("value", "Target prder parameter property value");
  return params;
}

DiscreteNucleationParsed::DiscreteNucleationParsed(const InputParameters & params) :
    DiscreteNucleationBase(params),
    _F_value(getParam<Real>("value"))
{
}

void
DiscreteNucleationParsed::computeProperties()
{
  // check if a nucleation event list is available for the current element
  const std::vector<Real> & nucleus = _map.nuclei(_current_elem);

  // calculate penalty
  for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
  {
    // clear penalty value
    if (_prop_F)
      (*_prop_F)[_qp] = 0.0;

    for (unsigned int i = 0; i < _nvar; ++i)
    {
      const unsigned ii = _op_index[i];

      // modify the penalty magnitude with the nucleus mask
      const Real penalty = _penalty * nucleus[_qp];

      // deviation from the target concentration
      Real dc = (*_args[ii])[_qp] - _op_values[i];

      // ignore above/below target values for min/max modes respectively
      if ((_penalty_mode == 1 && dc > 0.0) || (_penalty_mode == 2 && dc < 0.0))
        dc = 0.0;

      // build free energy correction
      if (_prop_F)
        (*_prop_F)[_qp] += dc * dc * penalty;

      // first derivative
      if (_prop_dF[ii])
        (*_prop_dF[ii])[_qp] = 2.0 * dc * penalty;

      // second derivatives
      for (unsigned int jj = ii; jj < _nvar; ++jj)
      {
        if (_prop_d2F[ii][jj])
          (*_prop_d2F[ii][jj])[_qp] = 2.0 * penalty;

        // third derivatives
        if (_third_derivatives)
          for (unsigned int kk = jj; kk < _nvar; ++kk)
            if (_prop_d3F[ii][jj][kk])
              (*_prop_d3F[ii][jj][kk])[_qp] = 0.0;
      }
    }
  }
}
