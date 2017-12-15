/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ALLENCAHNBASE_H
#define ALLENCAHNBASE_H

#include "ACBulk.h"

/**
 * AllenCahnBase implements the residual of the Allen-Cahn equation in a general
 * way that can be templated to a scalar or tensor mobility. It uses the Free
 * Energy function and derivatives provided by a DerivativeParsedMaterial to
 * computer the residual for the bulk part of the Allen-Cahn equation.
 */
template <typename T>
class AllenCahnBase : public ACBulk<T>
{
public:
  AllenCahnBase(const InputParameters & parameters);

  static InputParameters validParams();
  virtual void initialSetup();

protected:
  typedef typename ACBulk<T>::PFFunctionType PFFunctionType;
  virtual Real computeDFDOP(PFFunctionType type);
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  // Explicitly declare the use of the following members of the parent class
  // https://isocpp.org/wiki/faq/templates#nondependent-name-lookup-members
  using ACBulk<T>::_L;
  using ACBulk<T>::_i;
  using ACBulk<T>::_j;
  using ACBulk<T>::_qp;
  using ACBulk<T>::_var;
  using ACBulk<T>::_phi;
  using ACBulk<T>::_test;
  using ACBulk<T>::_coupled_moose_vars;

private:
  const unsigned int _nvar;
  const MaterialProperty<Real> & _dFdEta;
  const MaterialProperty<Real> & _d2FdEta2;

  std::vector<const MaterialProperty<Real> *> _d2FdEtadarg;
};

template <typename T>
InputParameters
AllenCahnBase<T>::validParams()
{
  InputParameters params = ACBulk<Real>::validParams();
  params.addClassDescription("Allen-Cahn Kernel that uses a DerivativeMaterial Free Energy");
  params.addRequiredParam<MaterialPropertyName>(
      "f_name", "Base name of the free energy function F defined in a DerivativeParsedMaterial");
  return params;
}

template <typename T>
AllenCahnBase<T>::AllenCahnBase(const InputParameters & parameters)
  : ACBulk<T>(parameters),
    _nvar(_coupled_moose_vars.size()),
    _dFdEta(this->template getMaterialPropertyDerivative<Real>("f_name", _var.name())),
    _d2FdEta2(
        this->template getMaterialPropertyDerivative<Real>("f_name", _var.name(), _var.name())),
    _d2FdEtadarg(_nvar)
{
  // Iterate over all coupled variables
  for (unsigned int i = 0; i < _nvar; ++i)
    _d2FdEtadarg[i] = &this->template getMaterialPropertyDerivative<Real>(
        "f_name", _var.name(), _coupled_moose_vars[i]->name());
}

template <typename T>
void
AllenCahnBase<T>::initialSetup()
{
  /**
   * Check if both the non-linear as well as the auxiliary variables variables
   * are coupled. Derivatives with respect to both types of variables contribute
   * the residual.
   */
  ACBulk<T>::initialSetup();
  this->template validateNonlinearCoupling<Real>("f_name");
  this->template validateDerivativeMaterialPropertyBase<Real>("f_name");
}

template <typename T>
Real
AllenCahnBase<T>::computeDFDOP(PFFunctionType type)
{
  switch (type)
  {
    case ACBulk<T>::Residual:
      return _dFdEta[_qp];

    case ACBulk<T>::Jacobian:
      return _d2FdEta2[_qp] * _phi[_j][_qp];
  }

  mooseError("Internal error");
}

template <typename T>
Real
AllenCahnBase<T>::computeQpOffDiagJacobian(unsigned int jvar)
{
  // get the coupled variable jvar is referring to
  const unsigned int cvar = this->mapJvarToCvar(jvar);

  // ARGH!!!!!!!! HOW?!
  return ACBulk<T>::computeQpOffDiagJacobian(jvar) +
         _L[_qp] * (*_d2FdEtadarg[cvar])[_qp] * _phi[_j][_qp] * _test[_i][_qp];
}

#endif // ALLENCAHNBASE_H
