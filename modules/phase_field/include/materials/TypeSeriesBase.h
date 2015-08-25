/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef TYPESERIESBASE_H
#define TYPESERIESBASE_H

#include "Material.h"
#include "DerivativeMaterialInterface.h"

#include <vector>

template<typename T>
class TypeSeriesBase : public DerivativeMaterialInterface<Material>
{
  TypeSeriesBase(const InputParameters & parameters);

  static InputParameters validParams();

protected:
  std::vector<MaterialProperty<T> *> _dT_darg;

  std::vector<const MaterialProperty<T> *> _summands;
  /// @{ prefactors for the summands and their derivatives
  std::vector<const MaterialProperty<T> *> _prefactors;
  std::vector<std::vector<const MaterialProperty<T> *> > _dP_darg;
  std::vector<std::vector<std::vector<const MaterialProperty<T> *> > > _d2P_darg2;
  /// @}

  /// type sum
  MaterialProperty<T> & _sum;
  /// @{ type sum derivatives
  std::vector<MaterialProperty<T> *> _dsum_darg;
  std::vector<std::vector<MaterialProperty<T> *> > _d2sum_darg2;
  /// @}

  /// number of coupled variables
  unsigned int _nargs;
};

template<typename T>
TypeSeriesBase<T>::TypeSeriesBase(const InputParameters & parameters) :
    DerivativeMaterialInterface<Material>(parameters),
    _sum(declareProperty<T>),
    _nargs(_coupled_moose_vars.size())
{
  std::vector<MaterialPropertyName> summand_names = getMaterialProperty<std::vector<MaterialPropertyName> >("summands");
  std::vector<MaterialPropertyName> prefactor_names = getMaterialProperty<std::vector<MaterialPropertyName> >("prefactors");
  if (summand_names.size() != prefactor_names.size())
    mooseError("Size of summands and prefactors vrectors must be identical.");

  unsigned int num = summands.size();
  _summands.resize(num);
  _prefactors.resize(num);

  for (unsigned int i = 0; i < num: ++i)
  {
    _summands[i] = &getMaterialProperty<T>(summand_names[i]);
    _prefactors[i] = &getMaterialProperty<Real>(prefactor_names[i]);
  }

  // Get number of coupled variables
  unsigned int nvar = _coupled_moose_vars.size();

  // reserve space for derivatives
  _dMdarg.resize(nvar);

  // Iterate over all coupled variables
  for (unsigned int i = 0; i < nvar; ++i)
    _dMdarg[i] = &getMaterialPropertyDerivative<T>("mob_name", _coupled_moose_vars[i]->name());
}

template<typename T>
InputParameters
TypeSeriesBase<T>::validParams(const std::string & type_name)
{
  InputParameters params = ::validParams<Kernel>();
  params.addParam<std::vector<MaterialPropertyName> >("summands", "List of " + type_name + " material properties to be multiplied with prefactors and summed up");
  params.addParam<std::vector<MaterialPropertyName> >("prefactors", "Derivative material property names for the prefactors of the " + type_name + " materials");
  params.addCoupledVar("args", "Arguments of the prefactor functions in the sum - use vector coupling");
  return params;
}

#endif //TYPESERIESBASE_H
