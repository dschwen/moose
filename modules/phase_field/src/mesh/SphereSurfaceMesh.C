#include "SphereSurfaceMesh.h"

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
  return params;
}

SphereSurfaceMesh::SphereSurfaceMesh(const InputParameters & parameters) :
    MooseMesh(parameters),
    _radius(getParam<Real>("radius")),
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
  using SphereFace = std::array<unsigned int, 6>;
  std::vector<SphereFace> face_list = {
    {1, 3, 4}, {0, 3, 4}, {0, 2, 4}, {1, 2, 4},
    {1, 3, 5}, {0, 3, 5}, {0, 2, 5}, {1, 2, 5}
  };
  face_list.reserve(faces);

  // edges (extra entry for pending nodes to be created)
  using SphereEdge = std::array<unsigned int, 3>;
  std::vector<SphereEdge> edge_list = {
    {0, 2}, {0, 3}, {0, 4}, {0, 5},
    {1, 2}, {1, 3}, {1, 4}, {1, 5},
    {2, 4}, {3, 4}, {2, 5}, {3, 5}
  };
  edge_list.reserve(edges);

  // build face list from edges
  for (auto i = beginIndex(edge_list); i < edge_list.size(); ++i)
  {
    SphereEdge & edge_i = edge_list[i];

    for (auto j = i + 1; j < edge_list.size(); ++j)
    {
      SphereEdge & edge_j = edge_list[j];

      // edges i and j connected?
      std::array<decltype(i), 2> kk;
      if (edge_i[0] == edge_j[0])
        kk = {1,1};
      else if (edge_i[1] == edge_j[0])
        kk = {0,1};
      else if (edge_i[0] == edge_j[1])
        kk = {1,0};
      else if (edge_i[1] == edge_j[1])
        kk = {0,0};
      else
        continue;

      for (auto k = j + 1; k < edge_list.size(); ++k)
      {
        SphereEdge & edge_k = edge_list[k];

        if ((edge_k[0] == edge_i[kk[0]] && edge_k[1] == edge_j[kk[1]]) ||
            (edge_k[1] == edge_i[kk[0]] && edge_k[0] == edge_j[kk[1]]))
          std::cout << i << ',' << j << ',' << k << '\n';
      }
    }
  }

  mooseError("done");

  // helper vector to link split edges
  std::vector<unsigned int> other_half(edges);

  // iterate
  unsigned int node_id = 6;
  for (unsigned int i = 0; i < _depth; ++i)
  {
    // 1. split edges (loop over edge set of previous iteration)
    const unsigned int num_edges_1 = edge_list.size();
    for (unsigned int j = 0; j < num_edges_1; ++j)
    {
      SphereEdge & edge = edge_list[j];

std::cout << node_id << " = " << edge[0] << " - " << edge[1] << '\n';

      // compute edge center snapped to surface
      Point center = *(mesh.node_ptr(edge[0])) + *(mesh.node_ptr(edge[1]));

      // add new second half edge
      other_half[j] = edge_list.size();
      edge_list.push_back({ edge[1], node_id });

      // modify existing edge to represent first half
      edge[1] = node_id;

      // add center node
      center *= _radius / center.norm();
      mesh.add_point(center, node_id++);
    }
    const unsigned int num_edges_2 = edge_list.size();

    // 2. split faces
    const unsigned int num_faces = face_list.size();
    for (unsigned int j = 0; j < num_faces; ++j)
    {
      SphereFace & face = face_list[j];

      // for each face find the two half edges (and the half nodes) adjacent to each corner.
      std::array<std::pair<unsigned int, unsigned int>, 3> halfway_nodes;

      // center triangle nodes (unique halfway nodes)
      SphereFace unique_halfway_nodes;
      unsigned int found = 0;

      // iterate over the first half edges
      for (unsigned int k = 0; k < num_edges_1; ++k)
      {
        // the edge entries will always contain the face corner ids in their first component
        SphereEdge & edge = edge_list[k];
        SphereEdge & other_edge = edge_list[other_half[k]];

        for (unsigned int l = 0; l < 3; ++l)
        {
          if (edge[0] == face[l])
          {
            unsigned int other_l;
            if (other_edge[0] == face[(l+1) % 3])
              other_l = (l+1) % 3;
            else if (other_edge[0] == face[(l+2) % 3])
              other_l = (l+2) % 3;
            else
              continue;

            halfway_nodes[l].first = edge[1];
            halfway_nodes[other_l].second = edge[1];

            // add halfway node if it is unique
            for (unsigned int m = 0; m < found; ++m)
              if (unique_halfway_nodes[m] == edge[1])
                continue;

            mooseAssert(found < 3, "Geometry error");
            unique_halfway_nodes[found++] = edge[1];
          }
        }
      }

      // for each original face node
      for (unsigned int l = 0; l < 3; ++l)
      {
        // add internal edge
        edge_list.push_back({ halfway_nodes[l].first, halfway_nodes[l].second });
std::cout << halfway_nodes[l].first << ',' << halfway_nodes[l].second << '\n';

        // add corner triangle
        face_list.push_back({ face[l], halfway_nodes[l].first, halfway_nodes[l].second });
std::cout << face[l] << ',' << halfway_nodes[l].first << ',' << halfway_nodes[l].second << '\n';
      }

      // modify existing face to center trangle, add three corner triangles
      face = unique_halfway_nodes;
std::cout << face[0] << ',' << face[1] << ',' << face[2] << '\n';
std::cout << '\n';
    }
  }

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
