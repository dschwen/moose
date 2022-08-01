//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

/**
 * Inteface class for MaterialProperty consumers to support deferred resolution of optional material
 * properties.
 */
class DeferredMaterialPropertyResolutionInterfaceBase
{
public:
  DeferredMaterialPropertyResolutionInterfaceBase() {}

  virtual void resolveAllProperties() = 0;
};

/**
 * Inteface class for MaterialProperty consumers to support deferred resolution of optional material
 * properties.
 */
template <class M>
class DeferredMaterialPropertyResolutionInterface
  : public DeferredMaterialPropertyResolutionInterfaceBase
{
  /**
   * Helper class for deferred getting of material properties after the construction
   * phase for materials. This enables "optional material properties" in materials.
   * It works by returning a reference to a pointer to a material property (rather
   * than a reference to the property value). The pointer will be set to point to
   * either an existing material property or to nullptr if the requested property
   * does not exist.
   */
  class DeferredMaterialPropertyProxyBase
  {
  public:
    DeferredMaterialPropertyProxyBase(const std::string & name, MaterialPropState state)
      : _name(name), _state(state)
    {
    }
    virtual ~DeferredMaterialPropertyProxyBase() {}
    virtual void resolve(M & material) = 0;

  private:
    const std::string _name;
    const MaterialPropState _state;
  };

  template <typename T, bool is_ad>
  class DeferredMaterialPropertyProxy : public DeferredMaterialPropertyProxyBase
  {
  public:
    DeferredMaterialPropertyProxy(const std::string & name, MaterialPropState state)
      : DeferredMaterialPropertyProxyBase(name, state)
    {
    }
    const GenericOptionalMaterialProperty<T, is_ad> & value() const { return _value; }
    void resolve(M & mpi) override;

  private:
    GenericOptionalMaterialProperty<T, is_ad> _value;
  };

public:
  DeferredMaterialPropertyResolutionInterface() {}
  virtual void resolveAllProperties();

protected:
  /// optional material properties (and zero material properties)
  std::vector<std::unique_ptr<DeferredMaterialPropertyProxyBase>> _deferred_property_proxies;
};

template <typename M>
template <typename T, bool is_ad>
void
DeferredMaterialPropertyResolutionInterface<M>::DeferredMaterialPropertyProxy<T, is_ad>::resolve(
    M & mpi)
{
  if (mpi.template hasGenericMaterialProperty<T, is_ad>(this->_name))
    switch (this->_state)
    {
      case MaterialPropState::CURRENT:
        _value.set(&mpi.template getGenericMaterialProperty<T, is_ad>(this->_name));
        break;

      case MaterialPropState::OLD:
        if constexpr (is_ad)
          mooseError("Old material properties are not available as AD");
        else
          _value.set(&mpi.template getMaterialPropertyOld<T>(this->_name));
        break;

      case MaterialPropState::OLDER:
        if constexpr (is_ad)
          mooseError("Older material properties are not available as AD");
        else
          _value.set(&mpi.template getMaterialPropertyOlder<T>(this->_name));
        break;
    }
}

template <typename M>
void
DeferredMaterialPropertyResolutionInterface<M>::resolveAllProperties()
{
  // deal with all fetched optional properties
  for (auto & proxy : _deferred_property_proxies)
    proxy->resolve(*this);
}
