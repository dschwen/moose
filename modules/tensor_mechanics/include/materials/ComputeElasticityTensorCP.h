/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef COMPUTEELASTICITYTENSORCP_H
#define COMPUTEELASTICITYTENSORCP_H

#include "ComputeElasticityTensorBase.h"
#include "ElementPropertyReadFile.h"
#include "RankTwoTensor.h"

/**
 * ComputeElasticityTensorCP defines an elasticity tensor material object for crystal plasticity.
 */
class ComputeElasticityTensorCP : public ComputeElasticityTensorBase
{
public:
  ComputeElasticityTensorCP(const InputParameters & parameters);

protected:
  virtual void computeQpElasticityTensor();

  virtual void getEulerRotations();

  /// Individual material information
  ElasticityTensorR4 _Cijkl;

  ///Element property read user object
  ///Presently used to read Euler angles -  see test
  const ElementPropertyReadFile * _read_prop_user_object;

  MaterialProperty<RealVectorValue> & _Euler_angles;

  /// Crystal Rotation Matrix
  MaterialProperty<RankTwoTensor> & _crysrot;

};

#endif //COMPUTEELASTICITYTENSORCP_H
