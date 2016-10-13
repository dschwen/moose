#include "SphereSurfaceMesh.h"

// libMesh includes
#include "libmesh/mesh_generation.h"

template<>
InputParameters validParams<SphereSurfaceMesh>()
{
  InputParameters params = validParams<MooseMesh>();
  MooseEnum elem_types("TRI3 TRI6", "TRI3");
  params.addParam<MooseEnum>("elem_type", elem_types, "The type of triangle element from libMesh to generate");
  params.addParam<Real>("radius", 1.0, "Sphere radius");
  params.addParam<unsigned int>("depth", 4, "Iteration steps in the triangle bisection construction");
  return params;
}

SphereSurfaceMesh::SphereSurfaceMesh(const InputParameters & parameters) :
    MooseMesh(parameters),
    _radius(getParam<unsigned int>("radius")),
    _depth(getParam<unsigned int>("depth"))
{
}

MooseMesh &
SphereSurfaceMesh::clone() const
{
  return *(new SphereSurfaceMesh(*this));
}

void
SphereSurfaceMesh::buildMesh()
{
  // Reserve nodes; start with initial octahedron
  unsigned int nodes = 6;
  unsigned int edges = 12;
  unsigned int faces = 8;

  /**
   *     /\        /\
   *    /  \      /__\
   *   /    \ -> /\  /\
   *  /______\  /__\/__\
   */
  for (unsigned int i = 0; i < _depth; ++i)
  {
    // 1 additional vertex for each edge
    nodes += edges;
    // duplicate edges and 3 additional edges per face
    edges = edges * 2 + faces * 3;
    // multiply faces by 4
    faces *= 4;
  }

  // set up mesh
  UnstructuredMesh & mesh = dynamic_cast<UnstructuredMesh &>(getMesh());
  mesh.set_mesh_dimension(2);
  mesh.set_spatial_dimension(3);

  // reserve space for objects in the mesh
  mesh.reserve_elem(faces);
  mesh.reserve_nodes(nodes);

  // build initial octahedron
  mesh.add_point(Point(-_radius, 0, 0), 0);
  mesh.add_point(Point( _radius, 0, 0), 1);
  mesh.add_point(Point(0, -_radius, 0), 2);
  mesh.add_point(Point(0,  _radius, 0), 3);
  mesh.add_point(Point(0, 0, -_radius), 4);
  mesh.add_point(Point(0, 0,  _radius), 5);

  /**
   *             5
   *             ^  3 (+y)
   *             z /
   *             |/
   *     0-------+-----x->1
   *            /|
   *           / |
   *          2  |
   *             4
   */

  // faces
  using SphereFace = std::array<unsigned int, 3>;
  std::vector<std::array<unsigned int, 3>> face_list = {
    {1, 3, 4}, {0, 3, 4}, {0, 2, 4}, {1, 2, 4},
    {1, 3, 5}, {0, 3, 5}, {0, 2, 5}, {1, 2, 5}
  };
  face_list.reserve(faces);

  // edges
  using SphereEdge = std::array<unsigned int, 2>;
  std::vector<SphereEdge> edge_list = {
    {0, 2}, {0, 3}, {0, 4}, {0, 5},
    {1, 2}, {1, 3}, {1, 4}, {1, 5},
    {2, 4}, {3, 4}, {2, 5}, {3, 5}
  };
  edge_list.reserve(edges);

  // iterate
  unsigned int node_id = 6;
  for (unsigned int i = 0; i < _depth; ++i)
  {
    // 1. split edges (loop over edge set of previous iteration)
    const unsigned int num_edges = edge_list.size();
    for (unsigned int j = 0; j < num_edges; ++j)
    {
      SphereEdge & edge = edge_list[j];

      // add new second half edge
      edge_list.push_back({edge[1], node_id});

      // modify existing edge to represent first half
      edge[1] == node_id;

      // add center node snapped to surface
      Point center = *(mesh.node_ptr(edge[0])) + *(mesh.node_ptr(edge[1]));
      center *= _radius / center.norm();
      mesh.add_point(center, node_id++);
    }

    // 2. split faces
    for (unsigned int j = 0; j < num_edges; ++j)
    {

      //for each face find the 3 edge pairs. Add three internal edges, modify existing face to center trangle, add three corner triangles

    }
  }
}
