/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef GRAINDATATRACKER_H
#define GRAINDATATRACKER_H

#include "GrainTracker.h"
#include "GrainDataUserObject.h"

/// base class for storage purposes
class GrainDataBase {};

/// data vector template calss
template<class T>
class GrainDataM<T> : public GrainDataBase
{
public:
  const T & getData(unsigned int grain_id) const;

protected:
  std::vector<T> _grain_data;
}

/**
 * GrainDataTracker is a GrainTracker object which maintains which maintains physical
 * parameters for individual grains.
 */
class GrainDataTracker : public GrainTracker
{
public:
  GrainDataTracker(const InputParameters & parameters);

  /// return data for selected grain
  template<typename T>
  const GrainData<T> & getColumn(const std::string & column) const;

protected:
  virtual void newGrainCreated(unsigned int new_grain_id) override;

private:
  /// per grain data
  std::map<std::string, std::pair<GrainDataUserObject *, GrainDataBase *>> _grain_data_columns;
};


GrainDataTracker::GrainDataTracker(const InputParameters & parameters) :
    GrainTracker(parameters)
{
}

template <typename T>
const GrainData<T> &
GrainDataTracker::getColumn<T>(const std::string & column) const
{
  auto colit = _grain_data_columns.find(column);
  if (colit != _grain_data_columns.end())
  {
    auto datptr = dynamic_cast<T *>(colit->second.second);
    if (dataptr != nullptr)
      return *dataptr
    else
      mooseError("The requested data column has the wrong type.");
  }
  else
    mooseError("Data column '" << column << "' not available.");
}

template <typename T>
const T &
GrainDataM<T>::getData(unsigned int grain_id) const
{
  mooseAssert(grain_id < _grain_data.size(), "Requested data for invalid grain index.");
  return _grain_data[grain_id];
}

void
GrainDataTracker::newGrainCreated(unsigned int new_grain_id)
{
  for (auto & col : _grain_data_columns)
    col.second.first->newGrainCreated(unsigned int new_grain_id, col.second.second);
}


template<class T>
const T &
GrainDataBase::getData<T>(unsigned int grain_id) const
{
  return T();
}

#endif // GRAINDATATRACKER_H
