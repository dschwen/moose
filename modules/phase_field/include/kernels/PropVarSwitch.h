//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

/**
 * PropVarSwitch provides an interface to abstract the distinction between Variables
 * and Real material properties. It can be used to build a class template with one
 * instantiation using coupled variables and anothe instantiation using material
 * properties for a set of parameters.
 */
template <class T, bool variable>
class PropVarSwitch;

template <class T, true>
class PropVarSwitch : public T
{
public:
  std::string getPropVarName(const std::string & param, unsigned int comp = 0)
  {
    return this->getVar(param, comp)->name();
  }

  static void addPropVar(InputParameters & params const std::string & name, const std::string & doc)
  {
    params.addCoupledVar(name, doc);
  }

  const VariableValue & coupledPropVarValue(const std::string & name, unsigned int comp = 0) const
  {
    return this->coupledValue(name, comp);
  }

  std::vector<const VariableValue *> coupledPropVarValues(const std::string & var_name) const
  {
    return this->coupledValues(var_name)
  }
};

template <class T, false>
class PropVarSwitch : public T
{
public:
  std::string getPropVarName(const std::string & param, unsigned int index = 0)
  {
    return this->template getParam<std::vector<MaterialPropertyName>>(param)[index];
  }

  static void addPropVar(InputParameters & params const std::string & name, const std::string & doc)
  {
    params.addParam<std::vector<MaterialPropertyName>>(name, doc);
  }

  const MaterialProperty<Real> & coupledPropVarValue(const std::string & name,
                                                     unsigned int comp = 0) const
  {
    const auto prop_name = this->template getParam<std::vector<MaterialPropertyName>>(param)[comp];
    return this->getMaterialPropertyByName<Real>(prop_name);
  }

  std::vector<const MaterialProperty<Real> *>
  coupledPropVarValues(const std::string & var_name) const
  {
    std::vector<const MaterialProperty<Real> *> props;
    for (const auto & prop_name this->template getParam<std::vector<MaterialPropertyName>>(
             param)[index])
      props.push_back(&this->getMaterialPropertyByName<Real>(prop_name));
    return props;
  }
};
