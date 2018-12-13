//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ALEKernel.h"

// MOOSE includes
#include "MooseVariable.h"

template <>
InputParameters
validParams<ALEKernel>()
{
  InputParameters params = validParams<Kernel>();
  params.addClassDescription("Sets up derivatives with respect to initial configuration");
  params.addCoupledVar("displacements", "mesh displacement variables for ");
  return params;
}

ALEKernel::ALEKernel(const InputParameters & parameters)
  : DerivativeMaterialInterface<Kernel>(parameters),
    _assembly_undisplaced(_fe_problem.assembly(_tid)),
    _var_undisplaced(
        _fe_problem.getStandardVariable(_tid, parameters.get<NonlinearVariableName>("variable"))),
    _ndisp(coupledComponents("displacements")),
    _disp_var(_ndisp, libMesh::invalid_uint),
    _grad_phi_undisplaced(_assembly_undisplaced.gradPhi()),
    _grad_test_undisplaced(_var_undisplaced.gradPhi()),
    _grad_disp_undisplaced(3),
    _coord_undisplaced(_assembly_undisplaced.coordTransformation())
{
  // fetch coupled gradients
  for (unsigned int i = 0; i < _ndisp; ++i)
  {
    _disp_var[i] = coupled("displacements", i);
    _grad_disp_undisplaced[i] = &coupledGradientUndisplaced("displacements", i);
  }

  // set unused dimensions to zero
  for (unsigned int i = _ndisp; i < 3; ++i)
    _grad_disp_undisplaced[i] = &_grad_zero;
}

void
ALEKernel::computeJacobian()
{
  _fe_problem.prepareShapes(_var.number(), _tid);
  Kernel::computeJacobian();
}

void
ALEKernel::computeOffDiagJacobian(MooseVariableFEBase & jvar)
{
  const auto jvar_num = jvar.number();
  if (jvar_num == _var.number())
    computeJacobian();
  else
  {
    _fe_problem.prepareShapes(jvar_num, _tid);

    // check if this is the off-diagonal Jacobian w.r.t. a displacement
    unsigned int disp_var;
    _jvar_is_disp = false;
    for (unsigned int i = 0; i < _ndisp; ++i)
      if (jvar_num == _disp_var[i])
      {
        disp_var = i;           // 0=x, 1=y, 2=z displacement
        _jvar_is_disp = true;   // this off-diagonal Jacobian is w.r.t. displacements
        _phiF.zero();           // zero out dF/dxj matrix
        precalculateResidual(); // for the dJxW/dxj derivative we need a residual value
        break;
      }

    prepareMatrixTag(_assembly, _var.number(), jvar_num);

    if (_local_ke.m() != _test.size() || _local_ke.n() != jvar.phiSize())
      return;

    precalculateOffDiagJacobian(jvar_num);

    // Real detF;
    for (_qp = 0; _qp < _qrule->n_points(); ++_qp)
    {
      // compute inverse of gradient tensor F if needed
      if (_jvar_is_disp)
      {
        // preparation
        for (unsigned int i = 0; i < 3; ++i)
          _matrix_F.fillColumn(i, (*_grad_disp_undisplaced[i])[_qp]);
        _matrix_F.addIa(1.0);
        _invF = _matrix_F.inverse();
        mooseAssert(_coord[_qp] == 1, "Only the XYZ coordinate system is supported.");
      }

      for (_i = 0; _i < _test.size(); ++_i)
      {
        // if jvar is a displacement we need to fetch the residual now (it is a function of test)
        Real res;
        if (_jvar_is_disp)
          res = computeQpResidual();

        for (_j = 0; _j < jvar.phiSize(); ++_j)
        {
          if (_jvar_is_disp)
          {
            _phiF.fillColumn(disp_var, _grad_phi_undisplaced[_j][_qp]);
            _phiF = _invF * _phiF;
            _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * _phiF.tr() * res;

            // this is used in the derived Kernel's off-diagonal Jacobian
            _phiInvF = -_phiF * _invF;
          }

          _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * computeQpOffDiagJacobian(jvar_num);
        }
      }
    }
    accumulateTaggedLocalMatrix();
  }
}

const VariableGradient &
ALEKernel::coupledGradientUndisplaced(const std::string & var_name, unsigned int comp)
{
  Coupleable::checkVar(var_name);
  if (!isCoupled(var_name)) // Return default 0
    return _default_gradient;

  coupledCallback(var_name, false);
  if (_c_nodal)
    mooseError(_c_name, ": Nodal variables do not have gradients");

  auto param = getParam<std::vector<VariableName>>(var_name);
  if (comp >= param.size())
    mooseError("component out of range");

  MooseVariable * var = &_fe_problem.getStandardVariable(_tid, param[comp]);

  if (!_coupleable_neighbor)
    return (_c_is_implicit) ? var->gradSln() : var->gradSlnOld();
  else
    return (_c_is_implicit) ? var->gradSlnNeighbor() : var->gradSlnOldNeighbor();
}
