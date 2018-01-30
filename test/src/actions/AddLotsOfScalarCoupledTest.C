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

#include "AddLotsOfScalarCoupledTest.h"
#include "Parser.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseEnum.h"
#include "AddVariableAction.h"
#include "Conversion.h"
#include "DirichletBC.h"

#include <sstream>
#include <stdexcept>

// libMesh includes
#include "libmesh/libmesh.h"
#include "libmesh/exodusII_io.h"
#include "libmesh/equation_systems.h"
#include "libmesh/nonlinear_implicit_system.h"
#include "libmesh/explicit_system.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/fe.h"

template <>
InputParameters
validParams<AddLotsOfScalarCoupledTest>()
{
  InputParameters params = validParams<AddVariableAction>();
  params.addRequiredParam<unsigned int>("number", "The number of variables to add kernels for");

  return params;
}

AddLotsOfScalarCoupledTest::AddLotsOfScalarCoupledTest(const InputParameters & params)
  : AddVariableAction(params)
{
}

void
AddLotsOfScalarCoupledTest::act()
{
  unsigned int number = getParam<unsigned int>("number");

  if (_current_task == "add_scalar_kernel")
  {
    for (unsigned int cur_num = 0; cur_num < number; cur_num++)
    {
      std::string var_name = name() + Moose::stringify(cur_num);

      // add coupling kernel
      {
        InputParameters params = _factory.getValidParams("ScalarCoupledTest");
        params.set<NonlinearVariableName>("variable") = var_name;

        if (cur_num > 0)
          params.set<std::vector<VariableName>>("c_down") = {name() +
                                                             Moose::stringify(cur_num - 1)};

        if (cur_num < number - 1)
          params.set<std::vector<VariableName>>("c_up") = {name() + Moose::stringify(cur_num + 1)};

        _problem->addScalarKernel("ScalarCoupledTest", var_name, params);
      }

      // add time kernel
      {
        InputParameters params = _factory.getValidParams("ODETimeDerivative");
        params.set<NonlinearVariableName>("variable") = var_name;
        _problem->addScalarKernel("ODETimeDerivative", "dt_" + var_name, params);
      }
    }
  }
}
