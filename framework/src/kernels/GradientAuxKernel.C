#include "GradientAuxKernel.h"

registerMooseObject("MooseApp", GradientAuxKernel);

InputParameters
GradientAuxKernel::validParams()
{
  InputParameters params = VectorAuxKernel::validParams();
  params.addClassDescription("Computes the gradient of a coupled variable for variable splitting");
  params.addRequiredCoupledVar("coupled_variable", "The variable whose gradient to compute");
  params.addParam<unsigned int>("component", 0, "Component of the gradient (0=x, 1=y, 2=z)");
  return params;
}

GradientAuxKernel::GradientAuxKernel(const InputParameters & parameters)
  : VectorAuxKernel(parameters),
    _grad_coupled(coupledGradient("coupled_variable")),
    _component(getParam<unsigned int>("component"))
{
}

RealVectorValue
GradientAuxKernel::computeValue()
{
  // Return the gradient at the current quadrature point
  return _grad_coupled[_qp];
}