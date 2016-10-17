#ifndef SPHERESURFACEMESH_H
#define SPHERESURFACEMESH_H

#include "MooseMesh.h"
#include <array>

class SphereSurfaceMesh;

template<>
InputParameters validParams<SphereSurfaceMesh>();

/**
 *
 */
class SphereSurfaceMesh : public MooseMesh
{
public:
  SphereSurfaceMesh(const InputParameters & parameters);
  SphereSurfaceMesh(const SphereSurfaceMesh & other_mesh) = default;

  // No copy
  SphereSurfaceMesh & operator=(const SphereSurfaceMesh & other_mesh) = delete;

  virtual MooseMesh & clone() const override;

  virtual void buildMesh() override;

protected:
  struct Triangle {
    std::array<std::size_t, 3> _node;
  };

  /// sphere radius
  const Real _radius;

  /// iteration depth
  const unsigned int _depth;
  const unsigned int _relax;
};

#endif // SPHERESURFACEMESH_H
