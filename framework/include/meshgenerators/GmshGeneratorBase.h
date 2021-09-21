//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MeshGenerator.h"
#include "gmsh.h"
#include "GmshUtils.h"

/**
 * Generates a line, square, or cube mesh with uniformly spaced or biased elements.
 */
class GmshGeneratorBase : public MeshGenerator
{
public:
  static InputParameters validParams();

  GmshGeneratorBase(const InputParameters & parameters);

  std::unique_ptr<MeshBase> generate() final;

protected:
  virtual void generateGeometry() = 0;

  // add physical groups for lower dimensional entities, this is needed in mapping boundary elements
  void addPhysicalGroups();
  void updateNodes(MeshBase & mesh);
  void updateElements(MeshBase & mesh);
  void updateBoundaryInfo(MeshBase & mesh);

  std::map<std::size_t, Node *> _map_tag_to_Node;
  std::vector<std::size_t> _nodeTags;

  // Map for node tag -> physical tag
  std::map<std::size_t, std::set<int>> _tag_node_to_physical;

  // Mesh dimension
  const int _dim;
};
