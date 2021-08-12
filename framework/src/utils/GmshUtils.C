//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "Moose.h"
#include "MooseError.h"
#include "GmshUtils.h"

// libMesh includes
#include "libmesh/node.h"
#include "libmesh/point.h"
#include "libmesh/node_elem.h"
#include "libmesh/edge_edge2.h"
#include "libmesh/edge_edge3.h"
#include "libmesh/face_tri3.h"
#include "libmesh/face_tri6.h"
#include "libmesh/face_quad4.h"
#include "libmesh/face_quad8.h"
#include "libmesh/face_quad9.h"
#include "libmesh/cell_tet4.h"
#include "libmesh/cell_tet10.h"
#include "libmesh/cell_hex8.h"
#include "libmesh/cell_hex27.h"
#include "libmesh/cell_prism6.h"
#include "libmesh/cell_prism15.h"
#include "libmesh/cell_prism18.h"
#include "libmesh/cell_pyramid5.h"
#include "libmesh/cell_pyramid13.h"
#include "libmesh/cell_pyramid14.h"

// Gmsh C++ API
#include "gmsh.h"

// STL includes
#include <map>
#include <vector>

namespace Moose
{

void
gmshToLibMesh(MeshBase & mesh)
{
  // Gmsh node data
  std::vector<std::size_t> nodeTags;
  std::vector<double> coord;
  std::vector<double> parametricCoord;

  // libmesh nodes
  std::map<std::size_t, Node *> nodes;

  // get and add nodes
  gmsh::model::mesh::getNodes(nodeTags, coord, parametricCoord);
  auto n_nodes = nodeTags.size();
  std::cout << "n_nodes = " << n_nodes << '\n';
  mooseAssert(n_nodes * 3 == coord.size(), "Wrong gmsh vector sizes returned");
  for (std::size_t i = 0; i < n_nodes; ++i)
    nodes[nodeTags[i]] = mesh.add_point(Point(coord[3 * i], coord[3 * i + 1], coord[3 * i + 2]), i);

  // Gmsh element data
  std::vector<int> elementTypes;
  std::vector<std::vector<std::size_t>> elementTags;
  std::vector<std::vector<std::size_t>> nodeTagsVec;

  // get and add elements
  gmsh::model::mesh::getElements(elementTypes, elementTags, nodeTagsVec);
  auto n_elem_types = elementTypes.size();
  for (std::size_t i = 0; i < n_elem_types; ++i)
  {
    if (gmshElemDim(elementTypes[i]) != 2)
      continue;

    auto n_nodes = gmshElemNodes(elementTypes[i]);
    mooseAssert(nodeTagsVec[i].size() % n_nodes == 0,
                "Number of nodes returned is not divisible by number of nodes per element for the "
                "given type");

    auto n_elems = nodeTagsVec[i].size() / n_nodes;
    for (std::size_t j = 0; j < n_elems; ++j)
    {
      auto elem = mesh.add_elem(gmshNewElem(elementTypes[i]));
      for (std::size_t k = 0; k < n_nodes; ++k)
      {
        // TODO: verify node orders
        elem->set_node(k) = nodes[nodeTagsVec[i][j * n_nodes + k]];
      }
    }
  }
}

Elem *
gmshNewElem(int type)
{
  switch (type)
  {
    case 1:
      return new Edge2;
    case 2:
      return new Tri3;
    case 3:
      return new Quad4;
    case 4:
      return new Tet4;
    case 5:
      return new Hex8;
    case 6:
      return new Prism6;
    case 7:
      return new Pyramid5;
    case 8:
      return new Edge3;
    case 9:
      return new Tri6;
    case 10:
      return new Quad9;
    case 11:
      return new Tet10;
    case 12:
      return new Hex27;
    case 13:
      return new Prism18;
    case 14:
      return new Pyramid14;
    case 15:
      return new NodeElem;
    case 16:
      return new Quad8;
    // case 17: Hex20
    case 18:
      return new Prism15;
    case 19:
      return new Pyramid13;
  }
  mooseError("Unsupported Gmsh element type ", type);
}

std::size_t
gmshElemNodes(int type)
{
  const std::array<std::size_t, 20> n_node_table = {0, 2,  3,  4,  4,  8, 6, 5, 3,  6,
                                                    9, 10, 27, 18, 14, 1, 8, 0, 15, 13};
  std::size_t n_node = 0;
  if (type > 0 && type < int(n_node_table.size()))
    n_node = n_node_table[type];

  if (n_node == 0)
    mooseError("Unsupported Gmsh element type ", type);

  return n_node;
}

int
gmshElemDim(int type)
{
  const std::array<int, 20> dim_table = {-1, 1, 2, 2, 3, 3, 3, 3,  1, 2,
                                         2,  3, 3, 3, 3, 0, 2, -1, 3, 3};
  int dim = -1;
  if (type > 0 && type < int(dim_table.size()))
    dim = dim_table[type];

  if (dim == -1)
    mooseError("Unsupported Gmsh element type ", type);

  return dim;
}

int
gmshAddRectangleLoop(Real xmin, Real xmax, Real ymin, Real ymax, Real lc)
{
  int p1 = gmsh::model::geo::addPoint(xmin, ymin, 0, lc);
  int p2 = gmsh::model::geo::addPoint(xmax, ymin, 0, lc);
  int p3 = gmsh::model::geo::addPoint(xmax, ymax, 0, lc);
  int p4 = gmsh::model::geo::addPoint(xmin, ymax, 0, lc);

  int l1 = gmsh::model::geo::addLine(p1, p2);
  int l2 = gmsh::model::geo::addLine(p2, p3);
  int l3 = gmsh::model::geo::addLine(p3, p4);
  int l4 = gmsh::model::geo::addLine(p4, p1);

  return gmsh::model::geo::addCurveLoop({l1, l2, l3, l4});
}

int
gmshAddCircleLoop(Real x, Real y, Real r, Real lc)
{
  int c = gmsh::model::geo::addPoint(x, y, 0, lc);
  int p1 = gmsh::model::geo::addPoint(x - r, y, 0, lc);
  int p2 = gmsh::model::geo::addPoint(x, y - r, 0, lc);
  int p3 = gmsh::model::geo::addPoint(x + r, y, 0, lc);
  int p4 = gmsh::model::geo::addPoint(x, y + r, 0, lc);

  int l1 = gmsh::model::geo::addCircleArc(p1, c, p2);
  int l2 = gmsh::model::geo::addCircleArc(p2, c, p3);
  int l3 = gmsh::model::geo::addCircleArc(p3, c, p4);
  int l4 = gmsh::model::geo::addCircleArc(p4, c, p1);

  return gmsh::model::geo::addCurveLoop({l1, l2, l3, l4});
}

} // namespace Moose
