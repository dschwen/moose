/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef CRYSTALPLASTICITYUOBASE_H
#define CRYSTALPLASTICITYUOBASE_H

#include "ElementUserObject.h"
#include "RankTwoTensor.h"

class CrystalPlasticityUOBase;

template<>
InputParameters validParams<CrystalPlasticityUOBase>();

/**
 * Crystal plasticity system userobject base class
 * The virtual functions written below must be
 * over-ridden in derived classes to provide actual values
 */

class CrystalPlasticityUOBase : public ElementUserObject
{
 public:
  CrystalPlasticityUOBase(const InputParameters & parameters);

  virtual ~CrystalPlasticityUOBase() {}

  void initialize() {}
  void execute() {}
  void finalize() {}
  void threadJoin(const UserObject &) {}

  /// Returns the size of variable
  virtual unsigned int variableSize() const;

 protected:

  unsigned int _variable_size;
};

#endif // CRYSTALPLASTICITYUOBASE_H
