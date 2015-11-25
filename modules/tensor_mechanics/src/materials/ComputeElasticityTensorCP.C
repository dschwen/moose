/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ComputeElasticityTensorCP.h"
#include "RotationTensor.h"

template<>
InputParameters validParams<ComputeElasticityTensorCP>()
{
  InputParameters params = validParams<ComputeElasticityTensorBase>();
  params.addClassDescription("Compute an elasticity tensor.");
  params.addRequiredParam<std::vector<Real> >("C_ijkl", "Stiffness tensor for material");
  params.addParam<MooseEnum>("fill_method", RankFourTensor::fillMethodEnum() = "symmetric9", "The fill method");
  params.addParam<UserObjectName>("read_prop_user_object","The ElementReadPropertyFile GeneralUserObject to read element specific property values from file");
  return params;
}

ComputeElasticityTensorCP::ComputeElasticityTensorCP(const InputParameters & parameters) :
    ComputeElasticityTensorBase(parameters),
    _Cijkl(getParam<std::vector<Real> >("C_ijkl"), (RankFourTensor::FillMethod)(int)getParam<MooseEnum>("fill_method")),
    _read_prop_user_object(isParamValid("read_prop_user_object") ? & getUserObject<ElementPropertyReadFile>("read_prop_user_object") : NULL),
    _Euler_angles(declareProperty<RealVectorValue>("Euler_angles")),
    _crysrot(declareProperty<RankTwoTensor>("crysrot"))
{
}

void
ComputeElasticityTensorCP::computeQpElasticityTensor()
{
  //Properties assigned at the beginning of every call to material calculation
  if ( _read_prop_user_object )
  {
    _Euler_angles[_qp](0) = _read_prop_user_object->getData( _current_elem , 0 );
    _Euler_angles[_qp](1) = _read_prop_user_object->getData( _current_elem , 1 );
    _Euler_angles[_qp](2) = _read_prop_user_object->getData( _current_elem , 2 );
  }

  getEulerRotations();

  RealTensorValue rot;
  for (unsigned int i = 0; i < 3; ++i)
    for (unsigned int j = 0; j < 3; ++j)
      rot(i,j) = _crysrot[_qp](i,j);

  _elasticity_tensor[_qp] = _Cijkl;
  _elasticity_tensor[_qp].rotate(rot);
}

void
ComputeElasticityTensorCP::getEulerRotations()
{
  Real phi1, phi, phi2;
  Real cp, cp1, cp2, sp, sp1, sp2;
  RankTwoTensor RT;
  Real pi = libMesh::pi;

  phi1 = _Euler_angles[_qp](0) * (pi/180.0);
  phi =  _Euler_angles[_qp](1) * (pi/180.0);
  phi2 = _Euler_angles[_qp](2) * (pi/180.0);

  cp1 = std::cos(phi1);
  cp2 = std::cos(phi2);
  cp = std::cos(phi);

  sp1 = std::sin(phi1);
  sp2 = std::sin(phi2);
  sp = std::sin(phi);

  RT(0,0) = cp1 * cp2 - sp1 * sp2 * cp;
  RT(0,1) = sp1 * cp2 + cp1 * sp2 * cp;
  RT(0,2) = sp2 * sp;
  RT(1,0) = -cp1 * sp2 - sp1 * cp2 * cp;
  RT(1,1) = -sp1 * sp2 + cp1 * cp2 * cp;
  RT(1,2) = cp2 * sp;
  RT(2,0) = sp1 * sp;
  RT(2,1) = -cp1 * sp;
  RT(2,2) = cp;

  _crysrot[_qp] = RT.transpose();
}
