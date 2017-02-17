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

#ifndef DUMPOBJECTSPROBLEM_H
#define DUMPOBJECTSPROBLEM_H

#include "FEProblem.h"

class DumpObjectsProblem;

template <>
InputParameters validParams<FEProblem>();

/**
 * Specialization of SubProblem for dumping generated objects as input file syntax
 */
class DumpObjectsProblem : public FEProblem
{
public:
  DumpObjectsProblem(const InputParameters & parameters);

  void addVariable(const std::string & var_name,
                   const FEType & type,
                   Real scale_factor,
                   const std::set<SubdomainID> * const active_subdomains = NULL) override;
  void addScalarVariable(const std::string & var_name,
                         Order order,
                         Real scale_factor = 1.,
                         const std::set<SubdomainID> * const active_subdomains = NULL) override;
  void addAuxVariable(const std::string & var_name,
                      const FEType & type,
                      const std::set<SubdomainID> * const active_subdomains = NULL) override;
  void addAuxScalarVariable(const std::string & var_name,
                            Order order,
                            Real scale_factor = 1.,
                            const std::set<SubdomainID> * const active_subdomains = NULL) override;

  void addFunction(std::string type, const std::string & name, InputParameters parameters) override;

  void addKernel(const std::string & type,
                 const std::string & name,
                 InputParameters parameters) override;
  void addNodalKernel(const std::string & type,
                      const std::string & name,
                      InputParameters parameters) override;
  void addScalarKernel(const std::string & type,
                       const std::string & name,
                       InputParameters parameters) override;
  void addBoundaryCondition(const std::string & type,
                            const std::string & name,
                            InputParameters parameters) override;
  void addConstraint(const std::string & type,
                     const std::string & name,
                     InputParameters parameters) override;
  void addAuxKernel(const std::string & type,
                    const std::string & name,
                    InputParameters parameters) override;
  void addAuxScalarKernel(const std::string & type,
                          const std::string & name,
                          InputParameters parameters) override;
  void addDiracKernel(const std::string & type,
                      const std::string & name,
                      InputParameters parameters) override;
  void addDGKernel(const std::string & type,
                   const std::string & name,
                   InputParameters parameters) override;
  void addInterfaceKernel(const std::string & type,
                          const std::string & name,
                          InputParameters parameters) override;
  void addInitialCondition(const std::string & type,
                           const std::string & name,
                           InputParameters parameters) override;
  void addMaterial(const std::string & type,
                   const std::string & name,
                   InputParameters parameters) override;

protected:
  void dumpHelper(const std::string & system,
                  const std::string & type,
                  const std::string & name,
                  InputParameters parameters);
};

#endif // DUMPOBJECTSPROBLEM_H
