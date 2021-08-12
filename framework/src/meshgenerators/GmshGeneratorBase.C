//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GmshGeneratorBase.h"
#include "GmshUtils.h"

InputParameters
GmshGeneratorBase::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  return params;
}

GmshGeneratorBase::GmshGeneratorBase(const InputParameters & parameters) : MeshGenerator(parameters)
{
}

std::unique_ptr<MeshBase>
GmshGeneratorBase::generate()
{
  // initialize libGmsh
  gmsh::initialize(0, nullptr);

  // call derived class method to build Gmsh geometry
  generateGeometry();

  // synchronize and mesh
  gmsh::model::geo::synchronize();
  gmsh::model::mesh::generate(2); // TODO params
  gmsh::model::mesh::recombine();

  // Have MOOSE construct the correct libMesh::Mesh object using Mesh block and CLI parameters.
  auto mesh = buildMeshBaseObject();

  // convert mesh data to libMesh
  Moose::gmshToLibMesh(*mesh);

  // deinitialize libGmsh
  gmsh::finalize();

  return dynamic_pointer_cast<MeshBase>(mesh);
}
