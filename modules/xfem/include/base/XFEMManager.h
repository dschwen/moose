/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef XFEMMANAGER_H
#ifndef XFEMMANAGER_H

#include "GlobalManager.h"

class XFEMInterface;

class XFEMManager : public GlobalManager
{
public:
  XFEMManager(FEProblem & fe_problem);

  /// Create XFEM controller object
  void initXFEM(MooseSharedPointer<XFEMInterface> xfem);

  /// Get a pointer to the XFEM controller object
  MooseSharedPointer<XFEMInterface> getXFEM() { return _xfem; }

  /// Find out whether the current analysis is using XFEM
  bool haveXFEM() { return _xfem != NULL; }

  /// Update the mesh due to changing XFEM cuts
  virtual bool updateMeshXFEM();

protected:
  /// Pointer to XFEM controller
  MooseSharedPointer<XFEMInterface> _xfem;
};

#endif //XFEMMANAGER_H
