/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CALPHADDERIVATIVES_H
#define CALPHADDERIVATIVES_H

#include "DerivativeFunctionMaterialBase.h"
#include "FunctionMaterialPropertyDescriptor.h"

class CALPHADDerivatives;

template<>
InputParameters validParams<CALPHADDerivatives>();

/**
 * Implement the CALPHAD free energy derivatives outlined in
 * http://mooseframework.org/wiki/PhysicsModules/PhaseField/CALPHAD/CalphadDerivations/
 */
class CALPHADDerivatives : public DerivativeFunctionMaterialBase
{
public:
  CALPHADDerivatives(const InputParameters & parameters);

protected:
  virtual void computeQpProperties();

  /// name of the CALPHAD free energy property (which is a function of sublattice concentrations)
  MaterialPropertyName _calphad_free_energy_name;

  /// convenience typedef for the material property descriptors
  typedef std::vector<FunctionMaterialPropertyDescriptor> ConstraintDescriptorList;

  /// Material property descriptors (obtained by parsing _mat_prop_expressions)
  ConstraintDescriptorList _constraint_descriptors;

  /// number of global concentrations
  unsigned int _n_global;

  /// index into _args for each global concentration
  std::vector<unsigned int> _global_index;

  /**
   * for each derivative \f$ n \f$, each global variable \f$ c \f$, each
   * sublattice concentration \f$ c_i \f$a pair of \f$ d^nF/dc^n_i \f$
   * and \f$ d^nc/dc^n_i \f$.
   */
  std::vector<std::vector<std::vector<std::pair<const MaterialProperty<Real> *, const MaterialProperty<Real> *> > > > _derivs;
};

#endif // CALPHADDERIVATIVES_H
