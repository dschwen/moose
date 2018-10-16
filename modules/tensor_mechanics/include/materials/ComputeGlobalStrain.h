//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef COMPUTEGLOBALSTRAIN_H
#define COMPUTEGLOBALSTRAIN_H

#include "Material.h"

// Forward Declarations
class ComputeGlobalStrain;
class GlobalStrainUserObjectInterface;
class RankTwoTensor;

template <>
InputParameters validParams<ComputeGlobalStrain>();

/**
 * ComputeGlobalStrain calculates the global strain tensor from the scalar variables
 */
class ComputeGlobalStrain : public Material
{
public:
  ComputeGlobalStrain(const InputParameters & parameters);

  virtual void initialSetup();

protected:
  virtual void computeProperties();

  /// Base name prepended to material property name
  std::string _base_name;

  ///@{ Scalar variables holding the global strain and the previous time step value
  const VariableValue & _scalar_global_strain;
  const VariableValue & _scalar_global_strain_old;
  ///@}

  ///@{ Tensor material property for the current and last time step global strain
  MaterialProperty<RankTwoTensor> & _global_strain;
  MaterialProperty<RankTwoTensor> & _global_strain_old;
  ///@}

  const GlobalStrainUserObjectInterface & _pst;
  const VectorValue<bool> & _periodic_dir;

  const unsigned int _dim;
  const unsigned int _ndisp;
};

#endif // COMPUTEGLOBALSTRAIN_H
