//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComputeGlobalStrain.h"
#include "GlobalStrainUserObjectInterface.h"

// MOOSE includes
#include "RankTwoTensor.h"

registerMooseObject("TensorMechanicsApp", ComputeGlobalStrain);

template <>
InputParameters
validParams<ComputeGlobalStrain>()
{
  InputParameters params = validParams<Material>();
  params.addClassDescription(
      "Material for storing the global strain values from the scalar variable");
  params.addParam<std::string>("base_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
  params.addCoupledVar("scalar_global_strain", "Scalar variable for global strain");
  params.addCoupledVar("displacements", "The name of the displacement variables");
  params.addRequiredParam<UserObjectName>("global_strain_uo",
                                          "The name of the GlobalStrainUserObject");

  return params;
}

ComputeGlobalStrain::ComputeGlobalStrain(const InputParameters & parameters)
  : Material(parameters),
    _base_name(isParamValid("base_name") ? getParam<std::string>("base_name") + "_" : ""),
    _scalar_global_strain(coupledScalarValue("scalar_global_strain")),
    _scalar_global_strain_old(coupledScalarValueOld("scalar_global_strain")),
    _global_strain(declareProperty<RankTwoTensor>(_base_name + "global_strain")),
    _global_strain_old(declareProperty<RankTwoTensor>(_base_name + "global_strain_old")),
    _pst(getUserObject<GlobalStrainUserObjectInterface>("global_strain_uo")),
    _periodic_dir(_pst.getPeriodicDirections()),
    _dim(_mesh.dimension()),
    _ndisp(coupledComponents("displacements"))
{
}

void
ComputeGlobalStrain::initQpStatefulProperties()
{
  mooseError("ComputeGlobalStrain material ",
             name(),
             " should not forced to be stateful. Instead use the ",
             _base_name + "global_strain_old",
             " property that "
             "accesses the old state of the global strain scalar variable!");
}

void
ComputeGlobalStrain::computeProperties()
{
  // convenience references to the tensors at quadrature point zero
  RankTwoTensor & strain = _global_strain[0];
  RankTwoTensor & strain_old = _global_strain_old[0];

  strain.fillFromScalarVariable(_scalar_global_strain);
  strain_old.fillFromScalarVariable(_scalar_global_strain_old);

  // zero out strain in non-periodic directions
  for (unsigned int dir = 0; dir < _dim; ++dir)
    if (!_periodic_dir(dir))
      for (unsigned int var = 0; var < _ndisp; ++var)
      {
        strain(dir, var) = 0.0;
        strain_old(dir, var) = 0.0;
      }

  // copy updated quadrature point zero value to the remaining quadrature points
  for (_qp = 1; _qp < _qrule->n_points(); ++_qp)
  {
    _global_strain[_qp] = strain;
    _global_strain_old[_qp] = strain_old;
  }
}
