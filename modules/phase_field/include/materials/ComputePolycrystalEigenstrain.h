/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef COMPUTEPOLYCRYSTALEIGENSTRAIN_H
#define COMPUTEPOLYCRYSTALEIGENSTRAIN_H

#include "ComputeEigenstrainBase.h"
#include "DerivativeMaterialInterface.h"
#include "GrainDataTracker.h"

//Forward Declarations
class ComputePolycrystalEigenstrain;
class EulerAngleProvider;

/**
 * Compute an evolving elasticity tensor coupled to a grain growth phase field model.
 */
class ComputePolycrystalEigenstrain : public DerivativeMaterialInterface<ComputeEigenstrainBase>
{
public:
  ComputePolycrystalEigenstrain(const InputParameters & parameters);

protected:
  virtual void computeQpEigenstrain();

  /// Grain tracker object
  const GrainDataTracker<RankTwoTensor> & _grain_tracker;

  /// Number of order parameters
  unsigned int _op_num;

  /// Order parameters
  std::vector<const VariableValue *> _vals;

  /// vector of elasticity tensor material properties
  std::vector<MaterialProperty<RankTwoTensor> *> _Deigenstrain;
};

#endif //COMPUTEPOLYCRYSTALEIGENSTRAIN_H
