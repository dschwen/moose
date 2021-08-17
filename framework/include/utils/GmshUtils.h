//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gmsh.h"
#include "libmesh/mesh_base.h"
#include "libmesh/elem.h"

namespace Moose
{
// construct a new libMesh element from a Gmsh type id
Elem * gmshNewElem(int type);

// Add rectangular loop
int gmshAddRectangleLoop(Real xmin,
                         Real xmax,
                         Real ymin,
                         Real ymax,
                         Real lc,
                         const std::vector<std::string> & names = {
                             "bottom", "right", "top", "left"});

// Add circular loop
int gmshAddCircleLoop(Real x,
                      Real y,
                      Real r,
                      Real lc,
                      const std::string & name = "circle");

}
