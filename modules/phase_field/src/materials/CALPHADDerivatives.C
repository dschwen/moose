/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "CALPHADDerivatives.h"

template<>
InputParameters validParams<CALPHADDerivatives>()
{
  InputParameters params = validParams<DerivativeFunctionMaterialBase>();
  params.addClassDescription("Meta-material to provide global concentration derivatives of a sublattice concentration based CALPHAD free energy");
  params.addParam<MaterialPropertyName>("calphad_free_energy", "Base name of the CALPHAD free energy function.");

  // All arguments of the free energies being summed
  params.addRequiredCoupledVar("global_args", "Global concentration variable arguments of the free energy functions being summed - use vector coupling");
  params.addParam<std::vector<std::string> >("constraint_functions", "Constraint functions for each of the global arguments - use the 'c(c1,c2,c3)' function material property syntax");

  params.addCoupledVar("displacement_gradients", "Vector of displacement gradient variables (see Modules/PhaseField/DisplacementGradients action)");
  return params;
}

CALPHADDerivatives::CALPHADDerivatives(const InputParameters & parameters) :
    DerivativeFunctionMaterialBase(parameters),
    _calphad_free_energy_name(getParam<MaterialPropertyName>("calphad_free_energy")),
    _n_global(coupledComponents("global_args")),
    _global_index(_n_global),
    _derivs(_third_derivatives ? 3 : 2)
{
  // get the constraint functions
  _constraint_descriptors = FunctionMaterialPropertyDescriptor::parseVector(getParam<std::vector<std::string> >("constraint_functions"), this);

  // check if we have one constraint per global concentration
  if (_constraint_descriptors.size() != _n_global)
    mooseError("CALPHADDerivatives needs one constraint function for each global concentration.");

  // get the arg indices of the global variables
  for (unsigned int j = 0; j < _n_global; ++j)
    _global_index[j] = argIndex(coupled("global_args", j));

  // fetch all derivatives
  for (unsigned int i = 0; i < _derivs.size(); ++i)
  {
    // iterate over all global concentrations
    _derivs[i].resize(_n_global);
    for (unsigned int j = 0; j < _n_global; ++j)
    {
      // iterate over all sublattice concentrations
      std::vector<VariableName> sublattice_conc = _constraint_descriptors[j].getDependentVariables();
      _derivs[i][j].resize(sublattice_conc.size());
      for (unsigned int k = 0; k < sublattice_conc.size(); ++k)
      {
        // make a copy of the constraint descriptors to take the derivatives
        FunctionMaterialPropertyDescriptor dcdci(_constraint_descriptors[j]);

        // take (i+1)th derivative
        for (unsigned int l = 0; l <= i; ++i)
          dcdci.addDerivative(sublattice_conc[k]);

        // (i+1)th derivative of the CALPHAD free energy
        const MaterialProperty<Real> * dFdci = &getMaterialPropertyDerivative<Real>(_calphad_free_energy_name, std::vector<VariableName>(i+1, sublattice_conc[k]));

        // store the pair of derivatives
        _derivs[i][j][k] = std::make_pair(dFdci, &dcdci.value());
      }
    }
  }
}

void
CALPHADDerivatives::computeQpProperties()
{
  for (unsigned int jj = 0; jj < _n_global; ++jj)
  {
    const unsigned int j = _global_index[jj];

    // set first derivatives
    if (_prop_dF[j]) {
      (*_prop_dF[j])[_qp] = 0;
      for (unsigned int k = 0; k < _derivs[0][jj].size(); ++k)
      {
        mooseAssert(_derivs[0][jj][k]->second)[_qp] != 0, "Derivative of the CALPHAD constraint function must not be zero.");
        (*_prop_dF[j])[_qp] += (_derivs[0][jj][k]->first)[_qp] / (_derivs[0][jj][k]->second)[_qp];
      }
    }

    // second derivatives
    if (_prop_d2F[j][j]) {
      (*_prop_d2F[j][j])[_qp] = 0;
      for (unsigned int k = 0; k < _derivs[1][jj].size(); ++k)
      {
        mooseAssert(_derivs[1][jj][k]->second)[_qp] != 0, "Derivative of the CALPHAD constraint function must not be zero.");
        (*_prop_d2F[j][j])[_qp] += (_derivs[1][jj][k]->first)[_qp] / (_derivs[1][jj][k]->second)[_qp];
      }
    }

    // third derivatives
    if (_prop_d3F[j][j][j]) {
      (*_prop_d3F[j][j][j])[_qp] = 0;
      for (unsigned int k = 0; k < _derivs[2][jj].size(); ++k)
      {
        mooseAssert(_derivs[2][jj][k]->second)[_qp] != 0, "Derivative of the CALPHAD constraint function must not be zero.");
        (*_prop_d3F[j][j][j])[_qp] += (_derivs[2][jj][k]->first)[_qp] / (_derivs[2][jj][k]->second)[_qp];
      }
    }
  }
}
