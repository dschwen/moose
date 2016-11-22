/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "RVEAntiperiodicTraction.h"

#include "RankTwoTensor.h"
#include "ElasticityTensorTools.h"
#include "MooseMesh.h"
#include <cmath>

template<>
InputParameters validParams<RVEAntiperiodicTraction>()
{
  InputParameters params = validParams<InterfaceKernel>();
  params.addRequiredParam<unsigned int>("component", "An integer corresponding to the direction the variable this kernel acts in. (0 for x, 1 for y, 2 for z)");
  params.addRequiredCoupledVar("disp_x", "The x displacement");
  params.addRequiredCoupledVar("disp_y", "The y displacement");
  params.addCoupledVar("disp_z", "The z displacement");
  params.set<std::string>("_moose_base") = "InterfaceKernel";
  return params;
}

RVEAntiperiodicTraction::RVEAntiperiodicTraction(const InputParameters & parameters) :
    InterfaceKernel(parameters),
    _component(getParam<unsigned int>("component")),
    _elasticity_tensor(getMaterialProperty<RankFourTensor>("elasticity_tensor")),
    _disp_x_var(coupled("disp_x")),
    _disp_y_var(coupled("disp_y")),
    _disp_z_var(_mesh.dimension() == 3 ? coupled("disp_z") : libMesh::invalid_uint),
    _disp_x(coupledValue("disp_x")),
    _disp_y(coupledValue("disp_y")),
    _disp_z(_mesh.dimension() == 3 ? coupledValue("disp_z") : _zero),
    _grad_disp_x(coupledGradient("disp_x")),
    _grad_disp_y(coupledGradient("disp_y")),
    _grad_disp_z(_mesh.dimension() == 3 ? coupledGradient("disp_z") : _grad_zero),
    _disp_x_var_test(*getVar("disp_x", 0)),
    _disp_y_var_test(*getVar("disp_y", 0)),
    _disp_z_var_test(*getVar("disp_z", 0)), //need to fix this so that we can go to 2D
    _grad_disp_x_neighbor_value(_disp_x_var_test.gradSlnNeighbor()),
    _grad_disp_y_neighbor_value(_disp_y_var_test.gradSlnNeighbor()),
    _grad_disp_z_neighbor_value(_mesh.dimension() == 3 ? _disp_z_var_test.gradSlnNeighbor() : _grad_zero)
{
  if (!parameters.isParamValid("boundary"))
  {
    mooseError("In order to use the RVEAntiperiodicTraction interface kernel, you must specify a boundary where it will live.");
  }
}

Real
RVEAntiperiodicTraction::computeRVEInterfaceResidualContrib(Moose::DGResidualType type,
   const RankFourTensor & c, const RankTwoTensor & a, const RankTwoTensor & an, unsigned int comp, const RealVectorValue & x)
{
  Real r = 0.0;
  switch (type)
  {
    case Moose::Element:
    {
      r = 0.0;
      for (unsigned int j = 0; j < 3; ++j)
        for (unsigned int k = 0; k < 3; ++k)
          for (unsigned int l = 0; l < 3; ++l)
          {
            // there is a 1/2 from the interface method (?) and 1/2 from the definition of linear elastic strain
            r += 0.25 * c(comp,j,k,l) * ( a(k,l) + a.transpose()(k,l) -  an(k,l) -  an.transpose()(k,l)) * x(j);
          }
      r *= _test[_i][_qp];
    }
      break;

    case Moose::Neighbor:
    {
      r = 0.0;
      for (unsigned int j = 0; j < 3; ++j)
        for (unsigned int k = 0; k < 3; ++k)
          for (unsigned int l = 0; l < 3; ++l)
          {
            r += 0.25 * c(comp,j,k,l) * ( a(k,l) + a.transpose()(k,l) -  an(k,l) -  an.transpose()(k,l)) * x(j);
          }
      r *= _test_neighbor[_i][_qp];
    }
      break;
  }
  return r;
}

Real
RVEAntiperiodicTraction::computeRVEInterfaceJacobianContrib(Moose::DGJacobianType type,
   unsigned int comp1, unsigned int comp2, RealVectorValue & x)
{
  Real j;
  switch (type)
  {
    case Moose::ElementElement:
    // the 0.25 becomes 0.5 due to the minor symmetry C_{ijkl} =  C_{ijlk}
    j = 0.5 * ElasticityTensorTools::momentNormalJacobian(_elasticity_tensor[_qp], comp1, comp2,
       _test[_i][_qp], _grad_phi[_j][_qp], x);
      break;

    case Moose::NeighborNeighbor:
    j = 0.5 * ElasticityTensorTools::momentNormalJacobian(_elasticity_tensor[_qp], comp1, comp2,
       _test_neighbor[_i][_qp], _grad_phi_neighbor[_j][_qp], x);
      break;

    case Moose::NeighborElement:
    j = 0.5 * ElasticityTensorTools::momentNormalJacobian(_elasticity_tensor[_qp], comp1, comp2,
       _test_neighbor[_i][_qp], _grad_phi[_j][_qp], x);
      break;

    case Moose::ElementNeighbor:
    j = 0.5 * ElasticityTensorTools::momentNormalJacobian(_elasticity_tensor[_qp], comp1, comp2,
       _test[_i][_qp], _grad_phi_neighbor[_j][_qp], x);
      break;
  }
  return j;
}


Real
RVEAntiperiodicTraction::computeQpResidual(Moose::DGResidualType type)
{
  Real res = 0.0;
  if (_component ==0)
  {
    // this line below here is "jerry-rigging" the normals... they aren't called right.
    // but this is approximately OK as long as there are no large deformations
    // and the mesh is square. What we should do is, put some sort of boundary
    // check here, then we could get shear periodicity too... allegedly.
    // if (boundary)
    // {
    //   then calculate normals properly. _normals[_qp] does not work!
    // }
    RealVectorValue n = (1.0, 0.0, 0.0);
    RankTwoTensor grad_tensor(_grad_u[_qp], _grad_disp_y[_qp], _grad_disp_z[_qp]);
    RankTwoTensor grad_neighbor_tensor(_grad_u_neighbor[_qp], _grad_disp_y_neighbor_value[_qp], _grad_disp_z_neighbor_value[_qp]);
    res = computeRVEInterfaceResidualContrib(type, _elasticity_tensor[_qp], grad_tensor, grad_neighbor_tensor, _component, n);
  }
  else if (_component == 1)
  {
    RealVectorValue n = (0.0, 1.0, 0.0);
    RankTwoTensor grad_tensor(_grad_disp_x[_qp], _grad_u[_qp], _grad_disp_z[_qp]);
    RankTwoTensor grad_neighbor_tensor(_grad_disp_x_neighbor_value[_qp], _grad_u_neighbor[_qp], _grad_disp_z_neighbor_value[_qp]);
    res = computeRVEInterfaceResidualContrib(type, _elasticity_tensor[_qp], grad_tensor, grad_neighbor_tensor, _component, n);
  }
  else if (_component == 2)
  {
    RealVectorValue n = (0.0, 0.0, 1.0);
    RankTwoTensor grad_tensor(_grad_disp_x[_qp], _grad_disp_y[_qp], _grad_u[_qp]);
    RankTwoTensor grad_neighbor_tensor(_grad_disp_x_neighbor_value[_qp], _grad_disp_y_neighbor_value[_qp], _grad_u_neighbor[_qp]);
    res = computeRVEInterfaceResidualContrib(type, _elasticity_tensor[_qp], grad_tensor, grad_neighbor_tensor, _component, n);
  }
  return res;
}

Real
RVEAntiperiodicTraction::computeQpJacobian(Moose::DGJacobianType type)
{
  Real jac = 0.0;
  // we need these if statements to be able to do the shear components
  // the shear component residual contributions will have on-diagonal contributions!
  if (_component == 0)
  {
    RealVectorValue n = (1.0, 0.0, 0.0);
    jac = computeRVEInterfaceJacobianContrib(type, _component, _component, n);
  }
  else if (_component == 1)
  {
    RealVectorValue n = (0.0, 1.0, 0.0);
    jac = computeRVEInterfaceJacobianContrib(type, _component, _component, n);
  }
  else if (_component == 2)
  {
    RealVectorValue n = (0.0, 0.0, 1.0);
    jac = computeRVEInterfaceJacobianContrib(type, _component, _component, n);
  }
  return jac;
}

//should have six off-diag jacobian components
Real
RVEAntiperiodicTraction::computeOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar)
{
  Real jac = 0.0;
  if (_component == 0)
  {
    //need to if loop this choice of n
    RealVectorValue n = (1.0, 0.0, 0.0);
    if (jvar == _disp_y_var)
    {
      unsigned int coupled_component = 1;
      jac = computeRVEInterfaceJacobianContrib(type, _component, coupled_component, n);
    }
    else if (jvar == _disp_z_var)
    {
      unsigned int coupled_component = 2;
      jac = computeRVEInterfaceJacobianContrib(type, _component, coupled_component, n);
    }
  }
  else if (_component == 1)
  {
    RealVectorValue n = (0.0, 1.0, 0.0);
    if (jvar == _disp_x_var)
    {
      unsigned int coupled_component = 0;
      jac = computeRVEInterfaceJacobianContrib(type, _component, coupled_component, n);
    }
    else if (jvar == _disp_z_var)
    {
      unsigned int coupled_component = 2;
      jac = computeRVEInterfaceJacobianContrib(type, _component, coupled_component, n);
    }
  }
  else if (_component == 2)
  {
    RealVectorValue n = (0.0, 0.0, 1.0);
    if (jvar == _disp_x_var)
    {
      unsigned int coupled_component = 0;
      jac = computeRVEInterfaceJacobianContrib(type, _component, coupled_component, n);
    }
    if (jvar == _disp_y_var)
    {
      unsigned int coupled_component = 1;
      jac = computeRVEInterfaceJacobianContrib(type, _component, coupled_component, n);
    }
  }
  return jac;
}
