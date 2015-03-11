/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GradientParsedMaterial.h"

template<>
InputParameters validParams<GradientParsedMaterial>()
{
  InputParameters params = ParsedMaterialHelper<FunctionMaterialBase>::validParams();
  params += validParams<ParsedMaterialBase>();
  params.addClassDescription("Parsed Function Material.");
  return params;
}

GradientParsedMaterial::GradientParsedMaterial(const std::string & name,
                               InputParameters parameters) :
    ParsedMaterialHelper<FunctionMaterialBase>(name, parameters, USE_MOOSE_NAMES),
    ParsedMaterialBase(name, parameters),
    _grad_F(declareProperty<RealGradient>("grad_" + _F_name))
{
  // Build function and optimize
  functionParse(_function,
                _constant_names, _constant_expressions,
                _mat_prop_names,
                _tol_names, _tol_values);

  // fetch argument gradients
  _grad_args.resize(_nargs);
  for (unsigned int i = 0; i < _nargs; ++i)
    _grad_args[i] = &(_arg_vars[i]->gradSln());

  // create all derivatives
  _func_dF.resize(_nargs);
  for (unsigned int i = 0; i < _nargs; ++i)
  {
    _func_dF[i] = new ADFunction(*_func_F);
    if (_func_dF[i]->AutoDiff(_arg_names[i]) != -1)
      mooseError("Failed to take first derivative w.r.t. "
                 << _arg_names[i]);
  }
}

void
GradientParsedMaterial::computeProperties()
{
  unsigned int i, j, k;
  Real a;

  for (_qp = 0; _qp < _qrule->n_points(); _qp++)
  {
    // fill the parameter vector, apply tolerances
    for (i = 0; i < _nargs; ++i)
      _func_params[i] = (*_args[i])[_qp];

    // set function value
    if (_prop_F)
      (*_prop_F)[_qp] = evaluate(_func_F);

    // compute gradient
    _grad_F[_qp] = 0;
    for (i = 0; i < _nargs; ++i)
      _grad_F[_qp] += (*_grad_args[i])[_qp] * evaluate(_func_dF[i]);
  }
}
