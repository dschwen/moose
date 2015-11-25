/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CRYSTALPLASTICITYSTATEVARIABLEGSS_H
#define CRYSTALPLASTICITYSTATEVARIABLEGSS_H

#include "CrystalPlasticityStateVariable.h"
#include "RankTwoTensor.h"
#include "CrystalPlasticitySlipResistance.h"

class CrystalPlasticityStateVariableGSS;

template<>
InputParameters validParams<CrystalPlasticityStateVariableGSS>();

/**
 * Phenomenological constitutive model state variable userobject class
 * The virtual functions written below must be
 * over-ridden in derived classes to provide actual values
 */

class CrystalPlasticityStateVariableGSS : public CrystalPlasticityStateVariable
{
 public:

   CrystalPlasticityStateVariableGSS(const InputParameters & parameters);

   virtual void initSlipSysProps(std::vector<Real> & val) const;

 protected:

   virtual void assignSlipSysRes(std::vector<Real> & val) const;

   virtual void getInitSlipSysRes(std::vector<Real> & val) const;

   std::vector<Real> _gprops;
};
#endif // CRYSTALPLASTICITYSTATEVARIABLE_H
