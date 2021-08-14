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
#include "libmesh/cell_hex20.h"
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

  // Gmsh model dimension
  auto model_dim = gmsh::model::getDimension();

  // Gmsh element data
  std::vector<int> elementTypes;
  std::vector<std::vector<std::size_t>> elementTags;
  std::vector<std::vector<std::size_t>> nodeTagsVec;

  // get and add elements
  gmsh::model::mesh::getElements(elementTypes, elementTags, nodeTagsVec);
  auto n_elem_types = elementTypes.size();
  for (std::size_t i = 0; i < n_elem_types; ++i)
  {
    std::string elementName;
    int dim;
    int order;
    int n_nodes;
    std::vector<double> localNodeCoord;
    int numPrimaryNodes;

    gmsh::model::mesh::getElementProperties(
        elementTypes[i], elementName, dim, order, n_nodes, localNodeCoord, numPrimaryNodes);

    // skip lower dimensional elements (faces/edges/points)
    if (dim < model_dim)
      continue;

    mooseAssert(nodeTagsVec[i].size() % n_nodes == 0,
                "Number of nodes returned is not divisible by number of nodes per element for the "
                "given type");

    // Node order mapping vectors contain the libmesh node index at the  Gmsh index position
    static const std::map<std::size_t, std::vector<std::size_t>> node_order_map = {
        {11 /* Tet10 */, {0, 1, 2, 3, 4, 5, 6, 7, 9, 8}},
        {17 /* Hex20 */, {0, 1, 2, 3, 4, 5, 6, 7, 8, 11, 12, 9, 13, 10, 14, 15, 16, 19, 17, 18}},
        {12 /* Hex27 */, {0,  1,  2,  3,  4,  5,  6,  7,  8,  11, 12, 9,  13, 10,
                          14, 15, 16, 19, 17, 18, 20, 21, 24, 22, 23, 25, 26}},
        {18 /* Prism15 */, {0, 1, 2, 3, 4, 5, 6, 8, 9, 7, 10, 11, 12, 14, 13}},
        {13 /* Prism18 */, {0, 1, 2, 3, 4, 5, 6, 8, 9, 7, 10, 11, 12, 14, 13, 15, 17, 16}},
        {19 /* Pyramid13 */, {0, 1, 2, 3, 4, 5, 8, 9, 6, 10, 7, 11, 12}},
        {14 /* Pyramid14 */, {0, 1, 2, 3, 4, 5, 8, 9, 6, 10, 7, 11, 12, 13}}};
    auto it = node_order_map.find(elementTypes[i]);

    auto n_elems = nodeTagsVec[i].size() / n_nodes;
    for (std::size_t j = 0; j < n_elems; ++j)
    {
      auto elem = gmshNewElem(elementTypes[i]);
      if (!elem)
        mooseError("Unsupported element type ", elementTypes[i], " (", elementName, ")");

      elem = mesh.add_elem(elem);

      // most element types have the same order in Gmsh and libMesh
      if (it == node_order_map.end())
        for (int k = 0; k < n_nodes; ++k)
          elem->set_node(k) = nodes[nodeTagsVec[i][j * n_nodes + k]];
      // some need to be shuffled though...
      else
        for (int k = 0; k < n_nodes; ++k)
          elem->set_node(it->second[k]) = nodes[nodeTagsVec[i][j * n_nodes + k]];
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
    case 17:
      return new Hex20;
    case 18:
      return new Prism15;
    case 19:
      return new Pyramid13;
  }
  return nullptr;
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
