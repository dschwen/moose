/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef KDTREEBASE_H
#define KDTREEBASE_H

// Moose includes
#include "MooseMesh.h"

#include "libmesh/nanoflann.hpp"
#include "libmesh/utility.h"

namespace MooseUtils
{
const Point & getKDTreePoint(const Point & p);
}

template <class T>
class KDTreeBase
{
public:
  KDTreeBase(std::vector<T> & master_points, unsigned int max_leaf_size);

  virtual ~KDTreeBase() = default;

  void neighborSearch(Point & query_point,
                      unsigned int patch_size,
                      std::vector<std::size_t> & return_index);

  /**
   * PointListAdaptor is required to use libMesh Point coordinate type with
   * nanoflann KDTreeBase library. The member functions within the PointListAdaptor
   * are used by nanoflann library.
   */
  template <unsigned int KDDim>
  class PointListAdaptor
  {
  private:
    const std::vector<T> & _pts;

  public:
    PointListAdaptor(const std::vector<T> & pts) : _pts(pts) {}

    /**
     * libMesh \p Point coordinate type
     */
    using coord_t = Real;
    /**
     * Must return the number of data points
     */
    inline size_t kdtree_get_point_count() const { return _pts.size(); }

    /**
     * Returns the distance between the vector "p1[0:size-1]"
     * and the data point with index "idx_p2" stored in the class
     */
    inline coord_t kdtree_distance(const coord_t * p1, const size_t idx_p2, size_t /*size*/) const
    {
      mooseAssert(idx_p2 <= _pts.size(),
                  "The point index should be less than"
                  "total number of points used to build"
                  "the KDTreeBase.");

      const Point & p2 = MooseUtils::getKDTreePoint(_pts[idx_p2]);

      coord_t dist = 0.0;

      for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
        dist += Utility::pow<2>(p1[i] - p2(i));

      return dist;
    }

    /**
     * Returns the dim'th component of the idx'th point in the class:
     * Since this is inlined and the "dim" argument is typically an immediate
     * value, the
     *  "if's" are actually solved at compile time.
     */
    inline coord_t kdtree_get_pt(const size_t idx, int dim) const
    {
      mooseAssert(dim < (int)LIBMESH_DIM,
                  "The required component number should be less than the LIBMESH_DIM.");
      mooseAssert(idx < _pts.size(),
                  "The index of the point should be less"
                  "than total number of points used to"
                  "construct the KDTreeBase.");

      const Point & p = MooseUtils::getKDTreePoint(_pts[idx]);

      return p(dim);
    }

    /**
     * Optional bounding-box computation. This function is called by the nanoflann library.
     * If the return value is false, the standard bbox computation loop in the nanoflann
     * library is activated.
     **/
    template <class BBOX>
    bool kdtree_get_bbox(BBOX & /* bb */) const
    {
      return false;
    }
  };

  using KdTreeT = nanoflann::KDTreeSingleIndexAdaptor<
      nanoflann::L2_Simple_Adaptor<Real, PointListAdaptor<LIBMESH_DIM>>,
      PointListAdaptor<LIBMESH_DIM>,
      LIBMESH_DIM>;

protected:
  PointListAdaptor<LIBMESH_DIM> _point_list_adaptor;
  std::unique_ptr<KdTreeT> _kd_tree;
};

template <class T>
KDTreeBase<T>::KDTreeBase(std::vector<T> & master_points, unsigned int max_leaf_size)
  : _point_list_adaptor(master_points),
    _kd_tree(libmesh_make_unique<KdTreeT>(
        LIBMESH_DIM, _point_list_adaptor, nanoflann::KDTreeSingleIndexAdaptorParams(max_leaf_size)))
{
  mooseAssert(_kd_tree != nullptr, "KDTreeBase was not properly initalized.");

  _kd_tree->buildIndex();
}

template <class T>
void
KDTreeBase<T>::neighborSearch(Point & query_point,
                              unsigned int patch_size,
                              std::vector<std::size_t> & return_index)
{
  // The query point has to be converted from a C++ array to a C array because nanoflann library
  // expects C arrays.
  const Real query_pt[] = {query_point(0), query_point(1), query_point(2)};

  return_index.resize(patch_size, std::numeric_limits<std::size_t>::max());
  std::vector<Real> return_dist_sqr(patch_size, std::numeric_limits<Real>::max());

  _kd_tree->knnSearch(&query_pt[0], patch_size, &return_index[0], &return_dist_sqr[0]);

  if (return_dist_sqr[0] == std::numeric_limits<Real>::max() ||
      return_index[0] == std::numeric_limits<std::size_t>::max())
    mooseError("Unable to find closest node!");
}

#endif // KDTREEBASE_H
