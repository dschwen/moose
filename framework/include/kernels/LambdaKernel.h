//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "MooseVariableInterface.h"

class LambdaKernel;

template <>
InputParameters validParams<LambdaKernel>();

class LambdaKernel : public Kernel
{
public:
  static InputParameters validParams();

  LambdaKernel(const InputParameters & parameters);

  /// Override this and call applyResidual with the residual Lambda
  virtual void computeResidual() override;

  /// Override this and call applyJacobian with the Jacobian Lambda
  virtual void computeJacobian() override;

  /// Override this and call applyJacobian with the off-diagonal Jacobian Lambda
  virtual void computeOffDiagJacobian(MooseVariableFEBase &) override;

protected:
  /// Compute this LambdaKernel's contribution to the residual
  template <typename T>
  void applyResidual(T lambda);

  /// Compute this LambdaKernel's contribution to the Jacobian
  template <typename T>
  void applyJacobian(T lambda);

  /// Compute this LambdaKernel's contribution to the off-diagonal Jacobian
  template <typename T>
  void applyOffDiagJacobian(MooseVariableFEBase & jvar, T lambda);

  ///@{ These won't get called
  virtual Real computeQpResidual() final { return 0; }
  virtual Real computeQpJacobian() final { return 0; }
  virtual Real computeQpOffDiagJacobian(unsigned int /*jvar*/) final { return 0; }
  ///@}
};

template <typename T>
void
LambdaKernel::applyResidual(T lambda)
{
  prepareVectorTag(_assembly, _var.number());

  precalculateResidual();
  for (_i = 0; _i < _test.size(); _i++)
    for (_qp = 0; _qp < _qrule->n_points(); _qp++)
      _local_re(_i) += _JxW[_qp] * _coord[_qp] * lambda();

  accumulateTaggedLocalResidual();

  if (_has_save_in)
  {
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _save_in)
      var->sys().solution().add_vector(_local_re, var->dofIndices());
  }
}

template <typename T>
void
LambdaKernel::applyJacobian(T lambda)
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  precalculateJacobian();
  for (_i = 0; _i < _test.size(); _i++)
    for (_j = 0; _j < _phi.size(); _j++)
      for (_qp = 0; _qp < _qrule->n_points(); _qp++)
        _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * lambda();

  accumulateTaggedLocalMatrix();

  if (_has_diag_save_in && !_sys.computingScalingJacobian())
  {
    DenseVector<Number> diag = _assembly.getJacobianDiagonal(_local_ke);
    Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
    for (const auto & var : _diag_save_in)
      var->sys().solution().add_vector(diag, var->dofIndices());
  }
}

template <typename T>
void
LambdaKernel::applyOffDiagJacobian(MooseVariableFEBase & jvar, T lambda)
{
  auto jvar_num = jvar.number();
  if (jvar_num == _var.number())
    computeJacobian();
  else
  {
    prepareMatrixTag(_assembly, _var.number(), jvar_num);

    // This (undisplaced) jvar could potentially yield the wrong phi size if this object is acting
    // on the displaced mesh
    auto phi_size = _sys.getVariable(_tid, jvar.number()).dofIndices().size();
    mooseAssert(
        phi_size * jvar.count() == _local_ke.n(),
        "The size of the phi container does not match the number of local Jacobian columns");

    if (_local_ke.m() != _test.size())
      return;

    precalculateOffDiagJacobian(jvar_num);
    if (jvar.count() == 1)
    {
      for (_i = 0; _i < _test.size(); _i++)
        for (_j = 0; _j < phi_size; _j++)
          for (_qp = 0; _qp < _qrule->n_points(); _qp++)
            _local_ke(_i, _j) += _JxW[_qp] * _coord[_qp] * lambda(jvar_num);
    }
    else
    {
      unsigned int n = phi_size;
      for (_i = 0; _i < _test.size(); _i++)
        for (_j = 0; _j < n; _j++)
          for (_qp = 0; _qp < _qrule->n_points(); _qp++)
          {
            RealEigenVector v =
                _JxW[_qp] * _coord[_qp] *
                computeQpOffDiagJacobianArray(static_cast<ArrayMooseVariable &>(jvar));
            for (unsigned int k = 0; k < v.size(); ++k)
              _local_ke(_i, _j + k * n) += v(k);
          }
    }

    accumulateTaggedLocalMatrix();
  }
}
