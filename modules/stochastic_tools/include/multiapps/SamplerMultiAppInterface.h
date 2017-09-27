/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef SAMPLERMULTIAPPINTERFACE_H
#define SAMPLERMULTIAPPINTERFACE_H

#include "InputParameters.h"

class SamplerMultiAppInterface;
class Sampler;

template <>
InputParameters validParams<SamplerMultiAppInterface>();

class SamplerMultiAppInterface
{
public:
  SamplerMultiAppInterface(Sampler & sampler) : _sampler(sampler) {}

  /**
   * Return the Sampler object for this MultiApp.
   */
  Sampler & getSampler() const { return _sampler; }

protected:
  /// Sampler to utilize for creating MultiApps
  Sampler & _sampler;
};

#endif // SAMPLERMULTIAPPINTERFACE_H
