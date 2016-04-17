/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "XFEMManager.h"

XFEMManager::XFEMManager(FEProblem & fe_problem) :
    GlobalManager(fe_problem),
    _app(fe_problem.getMooseApp())
{
  if (!_app.isRecovering())
  {
    if (haveXFEM() &&
        updateMeshXFEM())
      _console << "XFEM updated mesh on initializaton" << std::endl;
  }
}

void
XFEMManager::initXFEM(MooseSharedPointer<XFEMInterface> xfem)
{
  _xfem = xfem;
  _xfem->setMesh(&_mesh.getMesh());
  if (_displaced_mesh)
    _xfem->setSecondMesh(&_displaced_mesh->getMesh());
  _xfem->setMaterialData(&_material_data);

  unsigned int n_threads = libMesh::n_threads();
  for (unsigned int i = 0; i < n_threads; ++i)
  {
    _assembly[i]->setXFEM(_xfem);
    if (_displaced_problem != NULL)
      _displaced_problem->assembly(i).setXFEM(_xfem);
  }
}

bool
XFEMManager::updateMeshXFEM()
{
  bool updated = false;
  if (haveXFEM())
  {
    updated = _xfem->update(_time);
    if (updated)
    {
      meshChanged();
      _xfem->initSolution(_nl, _aux);
      restoreSolutions();
    }
  }
  return updated;
}
