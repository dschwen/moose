#include "SphereSurfaceMesh.h"
#include "RankTwoTensor.h"

// libMesh includes
#include "libmesh/unstructured_mesh.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/face_tri3.h"
#include "libmesh/face_tri6.h"

template<>
InputParameters validParams<SphereSurfaceMesh>()
{
  InputParameters params = validParams<MooseMesh>();
  MooseEnum elem_types("TRI3", "TRI3"); // TODO: TRI6
  params.addParam<MooseEnum>("elem_type", elem_types, "The type of triangle element from libMesh to generate");
  params.addParam<Real>("radius", 1.0, "Sphere radius");
  params.addParam<unsigned int>("depth", 4, "Iteration steps in the triangle bisection construction");
  params.addParam<unsigned int>("relax", 20, "Relaxation cycles to equilibrate triangle sizes");
  return params;
}

SphereSurfaceMesh::SphereSurfaceMesh(const InputParameters & parameters) :
    MooseMesh(parameters),
    _radius(getParam<Real>("radius")),
    _depth(getParam<unsigned int>("depth")),
    _relax(getParam<unsigned int>("relax"))
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
  mesh.clear();
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

  // faces (first 3 entries for the corner nodes, last three for the edge list)
  using SphereFace = std::array<unsigned int, 3>;
  std::vector<SphereFace> face_list = {
    {{1, 3, 4}}, {{0, 3, 4}}, {{0, 2, 4}}, {{1, 2, 4}},
    {{1, 3, 5}}, {{0, 3, 5}}, {{0, 2, 5}}, {{1, 2, 5}}
  };
  face_list.reserve(faces);

  // edges (extra entry for pending nodes to be created)
  using SphereEdge = std::pair<unsigned int, unsigned int>;
  std::vector<SphereEdge> edge_list = {
    {0, 2}, {0, 3}, {0, 4}, {0, 5},
    {1, 2}, {1, 3}, {1, 4}, {1, 5},
    {2, 4}, {3, 4}, {2, 5}, {3, 5}
  };
  edge_list.reserve(edges);

  // midpoint map
  std::map<std::pair<unsigned int, unsigned int>, unsigned int> midpoint_map;

  // iterate
  unsigned int node_id = 6;
  for (unsigned int i = 0; i < _depth; ++i)
  {
    // 1. split edges (loop over edge set of previous iteration)
    midpoint_map.clear();
    const unsigned int num_edges_1 = edge_list.size();
    for (unsigned int j = 0; j < num_edges_1; ++j)
    {
      SphereEdge & edge = edge_list[j];

      // make sure edges always go from the low node ID to the high node ID
      if (edge.first > edge.second)
        std::swap(edge.first, edge.second);

      // compute edge center snapped to surface
      Point center = *(mesh.node_ptr(edge.first)) + *(mesh.node_ptr(edge.second));

      // add to the midpoint map
      midpoint_map.insert(std::make_pair(edge, node_id));

      // add new second half edge
      edge_list.push_back({ edge.second, node_id });

      // modify existing edge to represent first half
      edge.second = node_id;

      // add center node
      center *= _radius / center.norm();
      mesh.add_point(center, node_id++);
    }

    // 2. split faces
    const unsigned int num_faces = face_list.size();
    for (unsigned int j = 0; j < num_faces; ++j)
    {
      SphereFace & face = face_list[j];
      SphereFace new_face;

      // iterator over corners
      for (unsigned int l = 0; l < 3; ++l)
      {
        // edges at that node
        auto edge_next = std::make_pair(std::min(face[l], face[(l+1)%3]), std::max(face[l], face[(l+1)%3]));
        auto edge_prev = std::make_pair(std::min(face[l], face[(l+2)%3]), std::max(face[l], face[(l+2)%3]));

        // the current corner and the two adjacent midpoints make the corner triangle
        mooseAssert(midpoint_map.find(edge_next) != midpoint_map.end(), "Midpoint not found " << edge_next.first << ' ' << edge_next.second);
        auto mid_next = midpoint_map[edge_next];
        mooseAssert(midpoint_map.find(edge_prev) != midpoint_map.end(), "Midpoint not found " << edge_prev.first << ' ' << edge_prev.second);
        auto mid_prev = midpoint_map[edge_prev];

        // insert corner triangle
        face_list.push_back({{ face[l], mid_next, mid_prev }});

        // insert adjacent inner edge
        edge_list.push_back(std::make_pair(mid_next, mid_prev));

        // store one of the midpoints for teh center tri
        new_face[l] = mid_prev;
      }

      // transform current face into center tri
      face = new_face;
    }
  }

  std::cout << "start moving nodes\n" << mesh.n_nodes() << " == " << nodes << '\n' << edge_list.size() << " == " << edges << '\n';

  // optimize nodes for equidistant distribution
  std::vector<RealVectorValue> center;
  for (unsigned int i = 0; i < _relax; ++i)
  {
    // compute center of surrounding nodes
    center.assign(nodes, RealVectorValue());
    for (auto j = beginIndex(edge_list); j < edge_list.size(); ++j)
    {
      const auto & edge = edge_list[j];
      center[edge.first] += *(mesh.node_ptr(edge.second));
      center[edge.second] += *(mesh.node_ptr(edge.first));
    }

    // move nodes to new centers and reproject (skip original octahedron corners)
    Real max_move = 0.0;
    for (unsigned int j = 6; j < nodes; ++j)
    {
      auto & node = *(mesh.node_ptr(j));
      RealVectorValue old_node = node;

      // move node
      node = center[j];

      // reproject to sphere surface
      node *= _radius / node.norm();

      // keep track of max displacement
      const Real move2 = (node - old_node).norm_sq();
      if (move2 > max_move)
        max_move = move2;
    }

    std::cout << "moved nodes " << std::sqrt(max_move)/_radius << '\n';
  }

  std::cout << "done moving nodes\n\n";

  // figure out element type
  MooseEnum elem_type_enum = getParam<MooseEnum>("elem_type");
  ElemType elem_type = Utility::string_to_enum<ElemType>(elem_type_enum);

  /**
   * Now that the face list is prepared and the nodes are all inserted we can
   * add the actual elements to the mesh.
   */
  const unsigned int num_faces = face_list.size();
  for (unsigned int j = 0; j < num_faces; ++j)
  {
    SphereFace & face = face_list[j];

    // fix winding order for the face

    switch(elem_type)
    {
      case TRI3:
      {
        Elem * elem = mesh.add_elem (new Tri3);
        for (unsigned int l = 0; l < 3; ++l)
          elem->set_node(l) = mesh.node_ptr(face[l]);
        break;
      }

      default:
        mooseError("Unsupported element type");
    }
  }

  mesh.prepare_for_use(/*skip_renumber =*/ false);
}
