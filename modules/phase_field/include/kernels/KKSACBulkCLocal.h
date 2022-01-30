//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "KKSACBulkCBase.h"

// Forward Declarations

/**
 * KKSACBulkBase child class for the phase concentration difference term
 * \f$ \frac{dh}{d\eta}\frac{dF_a}{dc_a}(c_a-c_b) \f$
 * in the the Allen-Cahn bulk residual.
 *
 * The non-linear variable for this Kernel is the order parameter 'eta'.
 */
class KKSACBulkCLocalIntermediate : public KKSACBulkCBase
{
public:
  static InputParameters validParams();

  KKSACBulkCLocalIntermediate(const InputParameters & parameters);

protected:
  /// phase a concentration
  const MaterialProperty<Real> & _ca;

  /// phase b concentration
  const MaterialProperty<Real> & _cb;
};

typedef KKSACBulkCBaseResidual<KKSACBulkCLocalIntermediate> KKSACBulkCLocal;
