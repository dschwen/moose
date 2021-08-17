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
gmshAddRectangleLoop(Real xmin,
                     Real xmax,
                     Real ymin,
                     Real ymax,
                     Real lc,
                     const std::vector<std::string> & names)
{
  int p1 = gmsh::model::geo::addPoint(xmin, ymin, 0, lc);
  int p2 = gmsh::model::geo::addPoint(xmax, ymin, 0, lc);
  int p3 = gmsh::model::geo::addPoint(xmax, ymax, 0, lc);
  int p4 = gmsh::model::geo::addPoint(xmin, ymax, 0, lc);

  int l1 = gmsh::model::geo::addLine(p1, p2);
  int l2 = gmsh::model::geo::addLine(p2, p3);
  int l3 = gmsh::model::geo::addLine(p3, p4);
  int l4 = gmsh::model::geo::addLine(p4, p1);

  int s1 = gmsh::model::addPhysicalGroup(1, {l1});
  gmsh::model::setPhysicalName(1, s1, names[0]);
  int s2 = gmsh::model::addPhysicalGroup(1, {l2});
  gmsh::model::setPhysicalName(1, s2, names[1]);
  int s3 = gmsh::model::addPhysicalGroup(1, {l3});
  gmsh::model::setPhysicalName(1, s3, names[2]);
  int s4 = gmsh::model::addPhysicalGroup(1, {l4});
  gmsh::model::setPhysicalName(1, s4, names[3]);

  return gmsh::model::geo::addCurveLoop({l1, l2, l3, l4});
}

int
gmshAddCircleLoop(
    Real x, Real y, Real r, Real lc, const std::string & name)
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

  int s = gmsh::model::addPhysicalGroup(1, {l1, l2, l3, l4});
  gmsh::model::setPhysicalName(1, s, name);

  // return gmsh::model::geo::addCurveLoop({s});
  return gmsh::model::geo::addCurveLoop({l1, l2, l3, l4});
}

} // namespace Moose
