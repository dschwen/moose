//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GmshGeneratorBase.h"

InputParameters
GmshGeneratorBase::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  params.addRequiredParam<int>("dim", "Mesh dimension");
  return params;
}

GmshGeneratorBase::GmshGeneratorBase(const InputParameters & parameters)
  : MeshGenerator(parameters), _dim(getParam<int>("dim"))
{
  if (_dim != 2 && _dim != 3)
    paramError("dim", "Mesh dimension should be 2 or 3");
}

std::unique_ptr<MeshBase>
GmshGeneratorBase::generate()
{
  // initialize libGmsh
  gmsh::initialize(0, nullptr);

  // call derived class method to build Gmsh geometry
  generateGeometry();

  // synchronize
  gmsh::model::geo::synchronize();

  // add tags to the boundary elements
  addPhysicalGroups();

  // generate mesh
  gmsh::model::mesh::generate(_dim);
  gmsh::model::mesh::recombine();

  // Have MOOSE construct the correct libMesh::Mesh object using Mesh block and CLI parameters.
  auto mesh = buildMeshBaseObject(_dim);

  // convert mesh data to libMesh
  updateNodes(*mesh);

  updateElements(*mesh);

  updateBoundaryInfo(*mesh);

  // mesh->allow_renumbering(false);
  mesh->prepare_for_use();

  // deinitialize libGmsh
  gmsh::finalize();

  return dynamic_pointer_cast<MeshBase>(mesh);
}

void
GmshGeneratorBase::addPhysicalGroups()
{
  // add physical groups for lower dimensional entities
  int model_dim = gmsh::model::getDimension();
  std::vector<std::pair<int, int>> entities;
  gmsh::model::getEntities(entities, model_dim - 1);
  for (auto e : entities)
  {
    // Dimension and tag of the entity:
    int dim = e.first, tag = e.second;

    std::vector<int> physicalTags;
    gmsh::model::getPhysicalGroupsForEntity(dim, tag, physicalTags);
    if (physicalTags.size() == 0)
      gmsh::model::addPhysicalGroup(dim, {tag});
  }
}

void
GmshGeneratorBase::updateNodes(MeshBase & mesh)
{
  _map_tag_to_Node.clear();
  _nodeTags.clear();
  _tag_node_to_physical.clear();

  // Gmsh node data
  std::vector<double> coord;
  std::vector<double> parametricCoord;

  // Get and add nodes to libmesh mesh
  gmsh::model::mesh::getNodes(_nodeTags, coord, parametricCoord);
  auto n_nodes = _nodeTags.size();
  mooseAssert(n_nodes * 3 == coord.size(), "Wrong gmsh vector sizes returned");
  for (std::size_t i = 0; i < n_nodes; ++i)
    _map_tag_to_Node[_nodeTags[i]] =
        mesh.add_point(Point(coord[3 * i], coord[3 * i + 1], coord[3 * i + 2]), i);

#ifdef DEBUG
  for (auto mm : _nodeTags)
    std::cout << mm << " ";
  std::cout << std::endl;
#endif

  // Get boundary node info and save in _tag_node_to_physical
  // Gmsh model dimension
  auto model_dim = gmsh::model::getDimension();

  // Gmsh model entities
  std::vector<std::pair<int, int>> entities;
  gmsh::model::getEntities(entities);
  for (auto e : entities)
  {
    // Dimension and tag of the entity:
    int dim = e.first, tag = e.second;

    // Gmsh element data
    std::vector<int> elemTypes;
    std::vector<std::vector<std::size_t>> elemTags, elemNodeTags;

    // Extract Boundary info from the lower dimensional entities
    if (dim != model_dim - 1)
      continue;

    // Also, only add physical tags for entities that belong to a valid physical group
    std::vector<int> physicalTags;
    gmsh::model::getPhysicalGroupsForEntity(dim, tag, physicalTags);

#ifdef DEBUG
    std::cout << "Physical tags = ";
    for (auto kk : physicalTags)
      std::cout << kk << " ";
    std::cout << std::endl;
#endif

    if (physicalTags.size())
    {
      // Get the mesh elements for the entity (dim, tag):
      gmsh::model::mesh::getElements(elemTypes, elemTags, elemNodeTags, dim, tag);
      for (std::size_t etype = 0; etype < elemTypes.size(); ++etype)
      {
        // Loop through nodes, save node tag and its physical tags in map
        for (auto nd_tag : elemNodeTags[etype])
          for (auto ph_tag : physicalTags)
            if (_tag_node_to_physical.find(nd_tag) == _tag_node_to_physical.end())
              _tag_node_to_physical.insert(std::make_pair(nd_tag, std::set<int>{ph_tag}));
            else
              _tag_node_to_physical[nd_tag].insert(ph_tag);
      }
    }
  }
}

void
GmshGeneratorBase::updateElements(MeshBase & mesh)
{
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
    int dim, order, n_nodes, numPrimaryNodes;
    std::vector<double> localNodeCoord;

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

#ifdef DEBUG
    std::cout << "Element type = " << elementTypes[i] << std::endl;
#endif

    auto n_elems = nodeTagsVec[i].size() / n_nodes;
    if (n_elems != elementTags[i].size())
      mooseError("Number of elements wrong.");

#ifdef DEBUG
    std::vector<std::size_t> ele_node_tags;
    for (auto eTag : elementTags[i])
    {
      gmsh::model::mesh::getElement(eTag, elementTypes[i], ele_node_tags);
      for (auto tag : ele_node_tags)
        std::cout << tag << " ";
      std::cout << std::endl;
    }
#endif

    for (std::size_t j = 0; j < n_elems; ++j)
    {
      auto elem = Moose::gmshNewElem(elementTypes[i]);
      if (!elem)
        mooseError("Unsupported element type ", elementTypes[i], " (", elementName, ")");

      elem = mesh.add_elem(elem);

      for (int k = 0; k < n_nodes; ++k)
      {
        int tag = nodeTagsVec[i][j * n_nodes + k];
        if (_map_tag_to_Node.find(tag) == _map_tag_to_Node.end())
          mooseError("Node tag ", tag, " does not exist in the Gmsh node tags");
        unsigned int p;
        if (it == node_order_map.end())
          p = k; // most element types have the same order in Gmsh and libMesh
        else
          p = it->second[k]; // some need to be shuffled though...
        elem->set_node(p) = _map_tag_to_Node[tag];
      }
    }
  }
}

void
GmshGeneratorBase::updateBoundaryInfo(MeshBase & mesh)
{
  // loop through all elements in libmesh mesh
  for (auto elem : as_range(mesh.elements_begin(), mesh.elements_end()))
    for (unsigned int i = 0; i < elem->n_sides(); ++i)
    {
      std::unique_ptr<const Elem> ele_side = elem->side_ptr(i);
      std::set<int> physical_tag;
      for (unsigned int j = 0; j < ele_side->n_nodes(); ++j)
      {
        const Node * nd = ele_side->node_ptr(j);
        if (nd->id() >= _nodeTags.size())
          mooseError("Node ID ", nd->id(), " larger than the node tags size.");
        std::size_t nd_tag = _nodeTags[nd->id()];
        std::set<int> temp = physical_tag;
        physical_tag.clear();

        if (_tag_node_to_physical.find(nd_tag) == _tag_node_to_physical.end())
        {
          temp.clear();
          break;
        }

        if (temp.size() == 0)
          physical_tag = _tag_node_to_physical[nd_tag];
        else
        {
          std::set_intersection(_tag_node_to_physical[nd_tag].begin(),
                                _tag_node_to_physical[nd_tag].end(),
                                temp.begin(),
                                temp.end(),
                                std::inserter(physical_tag, physical_tag.begin()));
        }
      }
      // add side to boundary, use the physical tag in Gmesh as boundary id in libmesh
      if (physical_tag.size())
        for (auto ph_tag : physical_tag)
          mesh.get_boundary_info().add_side(elem, i, (unsigned int)(ph_tag));
    }

  // update boundary name in mesh, use physical name in Gmesh as the boundary name in libmesh
  auto model_dim = gmsh::model::getDimension();
  gmsh::vectorpair dimTags;
  gmsh::model::getPhysicalGroups(dimTags, model_dim - 1);
  for (auto dim_tag : dimTags)
  {
    std::string boundary_name;
    gmsh::model::getPhysicalName(dim_tag.first, dim_tag.second, boundary_name);
    mesh.get_boundary_info().sideset_name(dim_tag.second) = boundary_name;
    mesh.get_boundary_info().nodeset_name(dim_tag.second) = boundary_name;
  }
}
