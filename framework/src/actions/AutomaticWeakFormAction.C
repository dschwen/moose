#include "AutomaticWeakFormAction.h"
#include "FEProblem.h"
#include "Factory.h"
#include "MooseError.h"
#include "libmesh/string_to_enum.h"
#include "StringExpressionParser.h"
#include "MooseUtils.h"
#include "ExpressionTransformer.h"
#include "StrongFormLowerer.h"
#include "ExpressionSimplifier.h"
#include "MooseAST.h"
#include <algorithm>

registerMooseAction("MooseApp", AutomaticWeakFormAction, "create_problem_complete");
registerMooseAction("MooseApp", AutomaticWeakFormAction, "add_variable");
registerMooseAction("MooseApp", AutomaticWeakFormAction, "add_kernel");
registerMooseAction("MooseApp", AutomaticWeakFormAction, "add_bc");

InputParameters
AutomaticWeakFormAction::validParams()
{
  InputParameters params = Action::validParams();

  params.addClassDescription(
      "Automatically generates weak form from strong form PDEs");

  params.addParam<std::string>(
      "expressions",
      "Intermediate expressions separated by semicolons (e.g., 'u = vec(disp_x, disp_y); strain = sym(grad(u))')");

  params.addRequiredParam<std::string>(
      "strong_forms",
      "Strong form equations separated by semicolons (e.g., 'c_t = Du*laplacian(c) - c; T_t = k*laplacian(T) + Q*c')");

  params.addParam<std::map<std::string, Real>>(
      "parameters", "Parameters used in the energy expression (e.g., kappa=1.0)");

  params.addRequiredParam<std::vector<std::string>>("variables",
                                                    "Primary variables for the problem");

  params.addParam<std::vector<std::string>>("coupled_variables", "Additional coupled variables");

  params.addParam<bool>(
      "enable_splitting", true, "Enable automatic variable splitting for higher-order derivatives");

  params.addParam<unsigned int>("max_fe_order", 1, "Maximum finite element order available");

  params.addParam<bool>("use_automatic_differentiation", true, "Use AD for Jacobian computation");

  params.addParam<bool>(
      "compute_jacobian_numerically", false, "Use finite differences for Jacobian");

  params.addParam<Real>("fd_epsilon", 1e-8, "Finite difference epsilon");

  // Debugging and verbose output options
  params.addParam<bool>("verbose", false,
                         "Enable verbose output showing all processing steps and kernels added");
  params.addParam<bool>("debug_print_expressions", false,
                         "Print all parsed expressions and intermediate AST");
  params.addParam<bool>("debug_print_derivatives", false,
                         "Print detailed derivative calculations step by step");
  params.addParam<bool>("debug_print_simplification", false,
                         "Show expression simplification steps");
  params.addParam<bool>("debug_print_weak_form", false,
                         "Print final weak form expressions for each variable");
  params.addParam<bool>("debug_print_jacobian", false,
                         "Print Jacobian expressions for debugging convergence issues");

  params.addParam<bool>("add_time_derivative", false, "Add time derivative term");

  params.addParam<std::string>(
      "time_derivative_type", "first_order", "Type of time derivative (first_order, second_order)");

  params.addParam<bool>("enable_stabilization", false, "Enable stabilization for the weak form");

  params.addParam<std::string>(
      "stabilization_type", "SUPG", "Type of stabilization (SUPG, GLS, PSPG)");

  params.addParam<Real>("stabilization_parameter", 0.1, "Stabilization parameter value");

  params.addParam<bool>("symmetrize_jacobian", false, "Symmetrize the Jacobian matrix");

  params.addParam<bool>("use_preconditioner", true, "Use preconditioning");

  params.addParam<std::string>("preconditioner_type", "default", "Type of preconditioner");

  params.addParam<bool>("adaptive_splitting", false, "Use adaptive variable splitting");

  params.addParam<Real>("splitting_tolerance", 1e-3, "Tolerance for adaptive splitting");

  params.addParam<bool>("output_weak_form", false, "Output the generated weak form to file");

  params.addParam<std::string>("weak_form_file", "weak_form.txt", "File to output weak form");

  params.addParam<bool>("validate_conservation", false, "Check conservation properties");

  params.addParam<bool>("check_stability", false, "Perform stability analysis");

  params.addParam<bool>("estimate_condition_number", false, "Estimate condition number of system");

  // Boundary condition control
  params.addParam<bool>("generate_natural_bcs", true,
                        "Automatically generate natural boundary conditions from energy functional");
  params.addParam<std::vector<BoundaryName>>("natural_bc_boundaries",
                                               "Boundaries to apply natural BCs (default: all boundaries without Dirichlet BCs)");
  params.addParam<std::vector<BoundaryName>>("exclude_bc_boundaries",
                                               "Boundaries to exclude from automatic natural BC generation");

  return params;
}

AutomaticWeakFormAction::AutomaticWeakFormAction(const InputParameters & params) : Action(params)
{
  // Initialize member variables
  _variable_names = getParam<std::vector<std::string>>("variables");
  _enable_splitting = getParam<bool>("enable_splitting");
  _max_fe_order = getParam<unsigned int>("max_fe_order");
  _use_automatic_differentiation = getParam<bool>("use_automatic_differentiation");
  _compute_jacobian_numerically = getParam<bool>("compute_jacobian_numerically");
  _fd_epsilon = getParam<Real>("fd_epsilon");
  _add_time_derivative = getParam<bool>("add_time_derivative");
  _time_derivative_type = getParam<std::string>("time_derivative_type");
  _enable_stabilization = getParam<bool>("enable_stabilization");
  _stabilization_type = getParam<std::string>("stabilization_type");
  _stabilization_parameter = getParam<Real>("stabilization_parameter");
  _symmetrize_jacobian = getParam<bool>("symmetrize_jacobian");
  _use_preconditioner = getParam<bool>("use_preconditioner");
  _preconditioner_type = getParam<std::string>("preconditioner_type");
  _adaptive_splitting = getParam<bool>("adaptive_splitting");
  _splitting_tolerance = getParam<Real>("splitting_tolerance");
  _output_weak_form = getParam<bool>("output_weak_form");
  _weak_form_file = getParam<std::string>("weak_form_file");
  _validate_conservation = getParam<bool>("validate_conservation");
  _check_stability = getParam<bool>("check_stability");
  _estimate_condition_number = getParam<bool>("estimate_condition_number");

  // Initialize debugging flags
  _verbose = getParam<bool>("verbose");
  _debug_print_expressions = getParam<bool>("debug_print_expressions");
  _debug_print_derivatives = getParam<bool>("debug_print_derivatives");
  _debug_print_simplification = getParam<bool>("debug_print_simplification");
  _debug_print_weak_form = getParam<bool>("debug_print_weak_form");
  _debug_print_jacobian = getParam<bool>("debug_print_jacobian");

  if (isParamValid("coupled_variables"))
    _coupled_variable_names = getParam<std::vector<std::string>>("coupled_variables");
}

void
AutomaticWeakFormAction::act()
{
  if (_verbose)
  {
    _console << "\n[AutomaticWeakForm] Processing task: " << _current_task << "\n";
    _console << "  Variable splitting: " << (_enable_splitting ? "ENABLED" : "DISABLED") << "\n";
  }

  if (_current_task == "create_problem_complete")
  {
    // Initialize utility objects (may be used for advanced features later)
    unsigned int dim = _problem->mesh().dimension();
    _expr_builder = std::make_unique<moose::automatic_weak_form::MooseExpressionBuilder>(dim);
    _weak_form_gen = std::make_unique<moose::automatic_weak_form::WeakFormGenerator>(dim);
    _split_analyzer =
        std::make_unique<moose::automatic_weak_form::VariableSplittingAnalyzer>(_max_fe_order, dim);
    _mixed_gen = std::make_unique<moose::automatic_weak_form::MixedFormulationGenerator>();
    _problem_analyzer = std::make_unique<moose::automatic_weak_form::VariationalProblemAnalyzer>();
  }

  if (_current_task == "add_variable")
  {
    parseStrongForms();
    deriveWeakForms();
    analyzeVariables();
    addPrimaryVariables();
  }
  else if (_current_task == "add_kernel")
  {
    if (_verbose)
      _console << "\n[AutomaticWeakForm] Adding kernels from strong forms\n";

    addTimeDerivativeKernels();
    addExpressionKernels();
  }
  else if (_current_task == "add_bc")
  {
    // Boundary conditions would be added here if needed
    // For now, users should specify BCs explicitly in their input files
  }

  if (_output_weak_form && _current_task == "add_kernel")
    writeWeakFormToFile();
}

void
AutomaticWeakFormAction::parseEnergyFunctional()
{
  buildEnergyFromType();

  if (!_energy_functional)
    mooseError("Failed to build energy functional");
}

void
AutomaticWeakFormAction::buildEnergyFromType()
{
  switch (_energy_type)
  {
    case EnergyType::EXPRESSION:
    {
      // Use the new string parser for arbitrary expressions
      moose::automatic_weak_form::StringExpressionParser parser(_problem->mesh().dimension());

      // Enable verbose mode if requested
      parser.setVerbose(_verbose);

      // Register parameters if provided
      if (isParamValid("parameters"))
      {
        const auto & params = getParam<std::map<std::string, Real>>("parameters");
        for (const auto & [name, value] : params)
          parser.setParameter(name, value);
      }

      // Define variables
      for (const auto & var_name : _variable_names)
        parser.defineVariable(var_name);
      for (const auto & var_name : _coupled_variable_names)
        parser.defineVariable(var_name);

      // Define intermediate expressions if provided
      if (isParamValid("expressions"))
      {
        const auto & expressions = getParam<std::vector<std::string>>("expressions");
        for (const auto & expr : expressions)
        {
          // Each expression should be of the form "name = expression"
          size_t eq_pos = expr.find('=');
          if (eq_pos != std::string::npos)
          {
            std::string name = expr.substr(0, eq_pos);
            std::string rhs = expr.substr(eq_pos + 1);

            // Trim whitespace
            name.erase(0, name.find_first_not_of(" \t"));
            name.erase(name.find_last_not_of(" \t") + 1);
            rhs.erase(0, rhs.find_first_not_of(" \t"));
            rhs.erase(rhs.find_last_not_of(" \t") + 1);

            parser.defineExpression(name, rhs);
          }
        }
      }

      // Handle multiple energy expressions for coupled systems
      if (isParamValid("energy_expressions"))
      {
        const auto & energy_exprs =
            getParam<std::map<std::string, std::string>>("energy_expressions");
        _multiple_energies = parser.parseMultipleEnergies(energy_exprs);
      }
      else if (!_energy_expression.empty())
      {
        // Single energy expression
        _energy_functional = parser.parse(_energy_expression);
      }
      else
      {
        mooseError(
            "Either energy_expression or energy_expressions must be provided for expression type");
      }
    }
    break;

    case EnergyType::DOUBLE_WELL:
      buildDoubleWellEnergy();
      break;

    case EnergyType::ELASTIC_LINEAR:
      buildElasticEnergy();
      break;

    case EnergyType::ELASTIC_NEOHOOKEAN:
      buildNeoHookeanEnergy();
      break;

    case EnergyType::SURFACE_ISOTROPIC:
      buildSurfaceEnergy();
      break;

    case EnergyType::CAHN_HILLIARD:
      buildCahnHilliardEnergy();
      break;

    case EnergyType::CAHN_HILLIARD_FOURTH_ORDER:
      buildFourthOrderCahnHilliardEnergy();
      break;

    case EnergyType::PHASE_FIELD_CRYSTAL:
      buildPhaseFieldCrystalEnergy();
      break;

    case EnergyType::NAVIER_STOKES:
      buildNavierStokesEnergy();
      break;

    default:
      mooseError("Unknown energy type");
  }
}

void
AutomaticWeakFormAction::buildDoubleWellEnergy()
{
  if (_variable_names.empty())
    mooseError("At least one variable required");

  auto c = _expr_builder->field(_variable_names[0]);
  Real height = getParam<Real>("well_height");
  _energy_functional = _expr_builder->doubleWell(c, height);
}

void
AutomaticWeakFormAction::buildElasticEnergy()
{
  if (_variable_names.size() < _problem->mesh().dimension())
    mooseError("Elastic energy requires displacement components");

  std::string base_name = _variable_names[0];
  if (base_name.find("disp_x") != std::string::npos)
    base_name = "disp";

  auto strain = _expr_builder->strain(base_name);
  Real lambda = getParam<Real>("elastic_lambda");
  Real mu = getParam<Real>("elastic_mu");

  _energy_functional = _expr_builder->elasticEnergy(strain, lambda, mu);
}

void
AutomaticWeakFormAction::buildNeoHookeanEnergy()
{
  if (_variable_names.size() < _problem->mesh().dimension())
    mooseError("Neo-Hookean energy requires displacement components");

  std::string base_name = _variable_names[0];
  if (base_name.find("disp_x") != std::string::npos)
    base_name = "disp";

  auto F = _expr_builder->deformationGradient(base_name);
  Real lambda = getParam<Real>("elastic_lambda");
  Real mu = getParam<Real>("elastic_mu");

  _energy_functional = _expr_builder->neoHookean(F, mu, lambda);
}

void
AutomaticWeakFormAction::buildSurfaceEnergy()
{
  if (_variable_names.empty())
    mooseError("At least one variable required");

  auto phi = _expr_builder->field(_variable_names[0]);
  Real gamma = getParam<Real>("surface_energy");

  _energy_functional = _expr_builder->surfaceEnergy(phi, gamma);
}

void
AutomaticWeakFormAction::buildCahnHilliardEnergy()
{
  if (_variable_names.empty())
    mooseError("At least one variable required");

  auto c = _expr_builder->field(_variable_names[0]);
  Real kappa = getParam<Real>("gradient_coefficient");
  Real height = getParam<Real>("well_height");

  auto bulk = _expr_builder->doubleWell(c, height);
  auto grad_c = moose::automatic_weak_form::grad(c, _problem->mesh().dimension());
  auto gradient =
      moose::automatic_weak_form::multiply(moose::automatic_weak_form::constant(0.5 * kappa),
                                           moose::automatic_weak_form::dot(grad_c, grad_c));

  _energy_functional = moose::automatic_weak_form::add(bulk, gradient);
}

void
AutomaticWeakFormAction::buildFourthOrderCahnHilliardEnergy()
{
  if (_variable_names.empty())
    mooseError("At least one variable required");

  auto c = _expr_builder->field(_variable_names[0]);
  Real kappa = getParam<Real>("gradient_coefficient");
  Real lambda = getParam<Real>("fourth_order_coefficient");
  Real height = getParam<Real>("well_height");

  auto bulk = _expr_builder->doubleWell(c, height);
  auto grad_c = moose::automatic_weak_form::grad(c, _problem->mesh().dimension());
  auto gradient =
      moose::automatic_weak_form::multiply(moose::automatic_weak_form::constant(0.5 * kappa),
                                           moose::automatic_weak_form::dot(grad_c, grad_c));
  auto fourth = _expr_builder->fourthOrderRegularization(c, lambda);

  _energy_functional =
      moose::automatic_weak_form::add(moose::automatic_weak_form::add(bulk, gradient), fourth);
}

void
AutomaticWeakFormAction::buildPhaseFieldCrystalEnergy()
{
  mooseError("Phase field crystal energy not yet implemented");
}

void
AutomaticWeakFormAction::buildNavierStokesEnergy()
{
  mooseError("Navier-Stokes energy not yet implemented");
}

void
AutomaticWeakFormAction::analyzeVariables()
{
  for (const auto & var_name : _variable_names)
  {
    VariableInfo info;
    info.name = var_name;
    info.family = "LAGRANGE";
    info.order = "FIRST";
    info.is_vector = false;
    info.is_tensor = false;
    info.components = 1;
    info.is_auxiliary = false;
    info.is_split = false;

    _variable_info[var_name] = info;
  }

  if (_enable_splitting)
  {
    setupVariableSplitting();
  }
}

void
AutomaticWeakFormAction::setupVariableSplitting()
{
  if (_verbose)
    _console << "\n[AutomaticWeakForm] Analyzing expression for variable splitting...\n";

  if (!_energy_functional)
  {
    return;
  }

  _split_variables = _split_analyzer->generateSplitVariables(_energy_functional);

  if (_verbose)
  {
    if (_split_variables.empty())
      _console << "  No variable splitting required (all derivatives within FE order "
               << _max_fe_order << ")\n";
    else
      _console << "  Found " << _split_variables.size() << " variables requiring splitting\n";
  }

  for (const auto & [name, sv] : _split_variables)
  {
    if (_verbose)
      _console << "  - Split variable: " << sv.name
               << " = " << sv.definition->toString() << "\n";
    VariableInfo info;
    info.name = sv.name;
    info.family = "LAGRANGE";
    info.order = "FIRST";
    info.is_vector = std::holds_alternative<moose::automatic_weak_form::VectorShape>(sv.shape);
    info.is_tensor = std::holds_alternative<moose::automatic_weak_form::TensorShape>(sv.shape);
    info.components = info.is_vector ? _problem->mesh().dimension() : 1;
    info.is_auxiliary = true;
    info.is_split = true;
    info.parent_variable = sv.original_variable;

    _variable_info[sv.name] = info;
  }
}

void
AutomaticWeakFormAction::addPrimaryVariables()
{
  for (const auto & var_name : _variable_names)
  {
    const auto & info = _variable_info[var_name];

    if (_problem->hasVariable(var_name))
      continue;

    InputParameters params = _factory.getValidParams("MooseVariable");
    params.set<MooseEnum>("family") = info.family;
    params.set<MooseEnum>("order") = info.order;

    _problem->addVariable("MooseVariable", var_name, params);
  }
}

void
AutomaticWeakFormAction::addSplitVariables()
{
  if (_verbose && !_split_variables.empty())
    _console << "\n[AutomaticWeakForm] Adding split variables to nonlinear system\n";

  for (const auto & [name, sv] : _split_variables)
  {
    // Create variable info if it doesn't exist
    if (_variable_info.find(sv.name) == _variable_info.end())
    {
      VariableInfo info;
      info.name = sv.name;
      info.family = "LAGRANGE";
      info.order = "FIRST";
      info.is_vector = std::holds_alternative<moose::automatic_weak_form::VectorShape>(sv.shape);
      info.is_tensor = std::holds_alternative<moose::automatic_weak_form::TensorShape>(sv.shape);
      info.components = info.is_tensor ? _problem->mesh().dimension() * _problem->mesh().dimension() : 1;
      info.is_auxiliary = false;  // Split variables are part of the nonlinear system
      info.is_split = true;
      info.parent_variable = sv.original_variable;
      _variable_info[sv.name] = info;
    }

    const auto & info = _variable_info[sv.name];

    if (_problem->hasVariable(sv.name))
    {
      continue;
    }

    // For tensor variables (Hessian), we need to handle them specially
    // In 1D, the Hessian is just a scalar (d²u/dx²)
    InputParameters params = _factory.getValidParams("MooseVariable");
    params.set<MooseEnum>("family") = info.family;
    params.set<MooseEnum>("order") = info.order;
    _problem->addVariable("MooseVariable", sv.name, params);

    if (_verbose)
      _console << "  - Added split variable '" << sv.name
               << "' as nonlinear variable (order " << sv.derivative_order
               << " derivative of '" << sv.original_variable << "')\n";
  }
}

void
AutomaticWeakFormAction::addKernels()
{
  // Re-generate split variables if needed (action state doesn't persist across tasks)
  if (_enable_splitting && _split_variables.empty())
  {
    if (!_energy_functional)
      parseEnergyFunctional();
    if (_energy_functional)
      _split_variables = _split_analyzer->generateSplitVariables(_energy_functional);
  }

  if (_verbose)
    _console << "\n========================================\n"
             << "[AutomaticWeakForm] Adding Kernels\n"
             << "========================================\n";

  // First add constraint kernels for split variables
  if (_enable_splitting && !_split_variables.empty())
    addSplitConstraintKernels();

  // Check if this is a transient problem and add time derivative kernels
  bool is_transient = _problem->isTransient();

  if (_verbose)
    _console << "Problem type: " << (is_transient ? "TRANSIENT" : "STEADY-STATE") << "\n";

  // Handle multiple energy expressions for coupled systems
  if (!_multiple_energies.empty())
  {
    if (_verbose)
      _console << "\nProcessing multiple energy expressions for coupled system\n";

    for (const auto & [var_name, energy] : _multiple_energies)
    {
      if (_verbose)
        _console << "\n--- Processing variable: " << var_name << " ---\n";

      // Add time derivative kernel for transient problems
      if (is_transient)
        addTimeDerivativeKernelForVariable(var_name);

      // Transform the energy if we have split variables
      moose::automatic_weak_form::NodePtr transformed_energy = energy;
      if (_enable_splitting && !_split_variables.empty())
      {
        moose::automatic_weak_form::ExpressionTransformer transformer(_split_variables, _problem->mesh().dimension());
        transformed_energy = transformer.transform(energy);

        if (_verbose && transformed_energy != energy)
          _console << "  Transformed energy for split variables\n";
      }

      auto weak_form = _weak_form_gen->generateWeakForm(transformed_energy, var_name);
      _weak_forms[var_name] = weak_form;

      if (_debug_print_weak_form && weak_form)
        _console << "  Weak form: " << weak_form->toString() << "\n";

      generateKernelForVariable(var_name, weak_form, transformed_energy);
    }
  }
  // Handle single energy expression
  else if (_energy_functional)
  {
    if (_verbose)
      _console << "\nProcessing single energy functional\n";

    if (_debug_print_expressions)
      _console << "  Energy: " << _energy_functional->toString() << "\n";

    for (const auto & var_name : _variable_names)
    {
      if (_verbose)
        _console << "\n--- Processing variable: " << var_name << " ---\n";

      // Add time derivative kernel for transient problems
      if (is_transient)
        addTimeDerivativeKernelForVariable(var_name);

      // Transform the energy if we have split variables
      moose::automatic_weak_form::NodePtr transformed_energy = _energy_functional;
      if (_enable_splitting && !_split_variables.empty())
      {
        if (_verbose)
        {
          _console << "  Transforming energy with " << _split_variables.size() << " split variables\n";
          for (const auto & [name, sv] : _split_variables)
          {
            _console << "    Split var: " << name << " = " << sv.name << " (order " << sv.derivative_order << ")\n";
          }
        }

        moose::automatic_weak_form::ExpressionTransformer transformer(_split_variables, _problem->mesh().dimension());
        transformed_energy = transformer.transform(_energy_functional);

        if (_verbose)
        {
          _console << "  Original energy: " << _energy_functional->toString() << "\n";
          _console << "  Transformed energy: " << transformed_energy->toString() << "\n";
        }
      }

      auto weak_form = _weak_form_gen->generateWeakForm(transformed_energy, var_name);
      _weak_forms[var_name] = weak_form;

      // Always print for info messages (existing behavior)
      _console << "Generated weak form for " << var_name << ": ";
      if (weak_form)
        _console << weak_form->toString() << "\n";
      else
        _console << "null\n";

      generateKernelForVariable(var_name, weak_form, transformed_energy);

      // Also generate kernels for split variables (inside the loop where transformed_energy exists)
      if (_enable_splitting && !_split_variables.empty())
      {
        if (_verbose)
          _console << "  Generating kernels for split variables\n";

        for (const auto & [name, sv] : _split_variables)
        {
          if (_verbose)
            _console << "    Generating kernel for split variable: " << sv.name << "\n";

          // Generate weak form for the split variable
          auto split_weak_form = _weak_form_gen->generateWeakForm(transformed_energy, sv.name);
          _weak_forms[sv.name] = split_weak_form;

          if (split_weak_form)
          {
            _console << "Generated weak form for split variable " << sv.name << ": " << split_weak_form->toString() << "\n";
            generateKernelForVariable(sv.name, split_weak_form, transformed_energy);
          }
          else if (_verbose)
          {
            _console << "      WARNING: No weak form generated for " << sv.name << "\n";
          }
        }
      }
    }
  }

  if (_verbose)
    _console << "\n========================================\n"
             << "[AutomaticWeakForm] Kernel Addition Complete\n"
             << "========================================\n";
}

void
AutomaticWeakFormAction::addSplitConstraintKernels()
{
  if (_verbose)
    _console << "\n[AutomaticWeakForm] Adding constraint kernels for split variables\n";

  for (const auto & [name, sv] : _split_variables)
  {
    if (_verbose)
      _console << "  - Adding constraint kernel for: " << sv.name
               << " = grad(" << sv.original_variable << ")\n";

    // For first-order splits (q = ∇u), add gradient constraint
    if (sv.derivative_order == 1)
    {
      // We'll add separate scalar constraints for each component
      // This is simpler than dealing with vector kernels
      std::vector<std::string> comp_names = {"x", "y", "z"};

      for (unsigned int comp = 0; comp < _problem->mesh().dimension(); ++comp)
      {
        std::string var_name = sv.name + "_" + comp_names[comp];
        std::string kernel_name = var_name + "_constraint";

        // Check if this component variable exists
        // (for now, we assume scalar split variables)
        if (!_problem->hasVariable(var_name))
        {
          // Add the component variable
          InputParameters var_params = _factory.getValidParams("MooseVariable");
          var_params.set<MooseEnum>("family") = "LAGRANGE";
          var_params.set<MooseEnum>("order") = "FIRST";
          _problem->addVariable("MooseVariable", var_name, var_params);

          if (_verbose)
            _console << "    - Added component variable: " << var_name << "\n";
        }

        // Add constraint kernel: q_x = ∂u/∂x
        InputParameters params = _factory.getValidParams("ScalarGradientConstraint");
        params.set<NonlinearVariableName>("variable") = var_name;
        params.set<std::vector<VariableName>>("u") = {sv.original_variable};
        params.set<unsigned int>("component") = comp;

        _problem->addKernel("ScalarGradientConstraint", kernel_name, params);

        if (_verbose)
          _console << "    - Added constraint kernel: " << kernel_name
                   << " (enforces " << var_name << " = d" << sv.original_variable
                   << "/d" << comp_names[comp] << ")\n";
      }
    }
    else if (sv.derivative_order == 2)
    {
      if (_verbose)
        _console << "    - Adding Laplacian constraint kernel for: " << sv.name << "\n";

      // Add Laplacian constraint: u_d2 = laplacian(u)
      // Weak form after integration by parts: ∫ u_d2 * v dx + ∫ grad(u) · grad(v) dx = 0
      std::string kernel_name = sv.name + "_laplacian_constraint";

      // Use the dedicated LaplacianConstraintKernel
      InputParameters params = _factory.getValidParams("LaplacianConstraintKernel");
      params.set<NonlinearVariableName>("variable") = sv.name;
      params.set<std::vector<VariableName>>("u") = {sv.original_variable};

      _problem->addKernel("LaplacianConstraintKernel", kernel_name, params);

      if (_verbose)
        _console << "    - Added Laplacian constraint: " << kernel_name
                 << " (enforces " << sv.name << " = laplacian(" << sv.original_variable << "))\n";
    }
  }
}

void
AutomaticWeakFormAction::addTimeDerivativeKernelForVariable(const std::string & var_name)
{
  std::string kernel_name = var_name + "_time_derivative";

  if (_verbose)
    _console << "\n[AutomaticWeakForm] Adding time derivative kernel for variable: " << var_name << "\n";

  // Use the specialized VariationalTimeDerivative kernel if automatic differentiation is enabled
  if (_use_automatic_differentiation)
  {
    if (_verbose)
      _console << "  - Using VariationalTimeDerivative with automatic differentiation\n";

    InputParameters params = _factory.getValidParams("VariationalTimeDerivative");
    params.set<NonlinearVariableName>("variable") = var_name;
    params.set<Real>("coefficient") = 1.0;  // Default coefficient, could be made configurable
    params.set<bool>("use_automatic_differentiation") = true;
    _problem->addKernel("VariationalTimeDerivative", kernel_name, params);
  }
  else
  {
    if (_verbose)
      _console << "  - Using standard TimeDerivative kernel\n";

    // Use standard TimeDerivative kernel
    InputParameters params = _factory.getValidParams("TimeDerivative");
    params.set<NonlinearVariableName>("variable") = var_name;
    _problem->addKernel("TimeDerivative", kernel_name, params);
  }

  if (_verbose)
    _console << "  - Kernel name: " << kernel_name << "\n";
}

void
AutomaticWeakFormAction::generateKernelForVariable(
    const std::string & var_name,
    const moose::automatic_weak_form::NodePtr & weak_form,
    const moose::automatic_weak_form::NodePtr & energy_expr)
{
  (void)weak_form;
  std::string kernel_name = generateKernelName(var_name);

  // Choose kernel type based on whether AD is enabled
  std::string kernel_type = _use_automatic_differentiation ? "ADVariationalKernelBase" : "VariationalKernelBase";

  if (_verbose)
    _console << "\n[AutomaticWeakForm] Generating kernel for variable: " << var_name << "\n"
             << "  - Kernel name: " << kernel_name << "\n"
             << "  - Kernel type: " << kernel_type << "\n";

  InputParameters params = _factory.getValidParams(kernel_type);

  // First, we need to couple all the other variables BEFORE setting parameters
  // This is required for MOOSE's coupling system to work
  // We do this by modifying the validParams to add coupled variables
  // But we can't modify validParams after getting them, so we need a different approach

  // The VariationalKernelBase should handle coupling internally based on the
  // coupled_variables parameter we'll pass

  params.set<NonlinearVariableName>("variable") = var_name;

  // Get the energy type string from the input
  const auto & energy_type_enum = getParam<MooseEnum>("energy_type");
  params.set<MooseEnum>("energy_type") = energy_type_enum;
  params.set<bool>("use_automatic_differentiation") = _use_automatic_differentiation;
  params.set<bool>("compute_jacobian_numerically") = _compute_jacobian_numerically;
  params.set<Real>("fd_eps") = _fd_epsilon;
  params.set<bool>("enable_variable_splitting") = _enable_splitting;
  params.set<unsigned int>("fe_order") = _max_fe_order;

  // Pass the energy expression - use transformed if splitting was applied
  if (_energy_type == EnergyType::EXPRESSION)
  {
    // If we have a transformed energy expression, convert it to string
    if (energy_expr)
    {
      std::string transformed_expr = energy_expr->toString();
      params.set<std::string>("energy_expression") = transformed_expr;
      if (_verbose)
        _console << "  - Passing transformed energy expression to kernel: " << transformed_expr << "\n";
    }
    else
    {
      params.set<std::string>("energy_expression") = _energy_expression;
      if (_verbose)
        _console << "  - Passing original energy expression to kernel: " << _energy_expression << "\n";
    }

    // Pass parameters if any
    if (isParamValid("parameters"))
      params.set<std::map<std::string, Real>>("parameters") =
          getParam<std::map<std::string, Real>>("parameters");
  }

  // For coupled problems, each variable's kernel needs the OTHER variables as coupled
  std::vector<VariableName> coupled_vars;

  // Add all variables from the energy expression except the current one
  for (const auto & name : _variable_names)
  {
    if (name != var_name)
      coupled_vars.push_back(name);
  }

  // Also add any explicitly specified coupled variables
  for (const auto & name : _coupled_variable_names)
  {
    // Don't add if it's the current variable or already in the list
    if (name != var_name && std::find(coupled_vars.begin(), coupled_vars.end(), name) == coupled_vars.end())
      coupled_vars.push_back(name);
  }

  if (!coupled_vars.empty())
  {
    // Set the coupled variables
    params.set<std::vector<VariableName>>("coupled_vars") = coupled_vars;

    if (_verbose)
    {
      _console << "  - Coupled variables: ";
      for (size_t i = 0; i < _coupled_variable_names.size(); ++i)
      {
        if (i > 0) _console << ", ";
        _console << _coupled_variable_names[i];
      }
      _console << "\n";
    }
  }

  if (_verbose)
  {
    _console << "  - Automatic differentiation: " << (_use_automatic_differentiation ? "ENABLED" : "DISABLED") << "\n";
    if (_enable_splitting)
      _console << "  - Variable splitting: ENABLED (max FE order: " << _max_fe_order << ")\n";
  }

  _problem->addKernel(kernel_type, kernel_name, params);

  if (_verbose)
    _console << "  - Kernel successfully added to problem\n";
}

void
AutomaticWeakFormAction::addAuxKernels()
{
  if (_verbose)
    _console << "\n[AutomaticWeakForm] Adding auxiliary kernels for split variables\n";

  // For each split variable, add an auxiliary kernel to compute it
  for (const auto & [name, sv] : _split_variables)
  {
    if (_verbose)
      _console << "  - Adding kernel for split variable: " << sv.name
               << " (order " << sv.derivative_order << " derivative of "
               << sv.original_variable << ")\n";

    // For gradient computation (order 1), use GradientAuxKernel
    if (sv.derivative_order == 1)
    {
      // Check if the split variable is a vector (gradient of scalar)
      if (std::holds_alternative<moose::automatic_weak_form::VectorShape>(sv.shape))
      {
        // For vector auxiliary variables, we need to add component kernels
        std::vector<std::string> component_names = {"x", "y", "z"};
        for (unsigned int comp = 0; comp < _problem->mesh().dimension(); ++comp)
        {
          std::string kernel_name = sv.name + "_" + component_names[comp] + "_gradient";
          InputParameters params = _factory.getValidParams("GradientAuxKernel");

          // Set the vector auxiliary variable and component
          params.set<AuxVariableName>("variable") = sv.name;
          params.set<std::vector<VariableName>>("coupled_variable") = {sv.original_variable};
          params.set<unsigned int>("component") = comp;

          _problem->addAuxKernel("GradientAuxKernel", kernel_name, params);

          if (_verbose)
            _console << "    Added component " << component_names[comp] << " kernel: " << kernel_name << "\n";
        }
      }
    }
    // For higher-order derivatives, we'd need specialized kernels
    else if (sv.derivative_order == 2)
    {
      // This would compute the gradient of a gradient (Hessian)
      // Would need a HessianAuxKernel or similar
      if (_verbose)
        _console << "    WARNING: Second-order derivatives not yet fully implemented\n";
    }
  }

  if (_verbose && _split_variables.empty())
    _console << "  No split variables needed (all derivatives within FE order)\n";
}

void
AutomaticWeakFormAction::addBoundaryConditions()
{
  // Check if natural BC generation is enabled
  bool generate_natural_bcs = getParam<bool>("generate_natural_bcs");
  if (!generate_natural_bcs)
  {
    if (_verbose)
      _console << "\n[AutomaticWeakForm] Natural BC generation disabled\n";
    return;
  }

  // Skip if no energy functional
  if (!_energy_functional && _multiple_energies.empty())
    return;

  if (_verbose)
    _console << "\n[AutomaticWeakForm] Extracting and adding natural boundary conditions\n";

  // Transform energy if we have split variables
  moose::automatic_weak_form::NodePtr transformed_energy = _energy_functional;
  if (_enable_splitting && !_split_variables.empty() && _energy_functional)
  {
    moose::automatic_weak_form::ExpressionTransformer transformer(_split_variables, _problem->mesh().dimension());
    transformed_energy = transformer.transform(_energy_functional);
  }

  // Process each variable
  std::vector<std::string> vars_to_process = _variable_names;

  // Also process split variables if they exist
  for (const auto & [name, sv] : _split_variables)
    vars_to_process.push_back(sv.name);

  for (const auto & var_name : vars_to_process)
  {
    if (_verbose)
      _console << "\n--- Extracting boundary terms for variable: " << var_name << " ---\n";

    // Get the appropriate energy expression
    moose::automatic_weak_form::NodePtr energy_to_use = transformed_energy;
    if (!_multiple_energies.empty() && _multiple_energies.count(var_name) > 0)
      energy_to_use = _multiple_energies.at(var_name);

    if (!energy_to_use)
    {
      if (_verbose)
        _console << "  No energy expression for " << var_name << ", skipping boundary terms\n";
      continue;
    }

    // Compute contributions (includes boundary terms)
    auto contributions = _weak_form_gen->computeContributions(energy_to_use, var_name);

    if (contributions.boundary_terms.empty())
    {
      if (_verbose)
        _console << "  No boundary terms found (pure bulk energy or Dirichlet-only problem)\n";
      continue;
    }

    if (_verbose)
      _console << "  Found " << contributions.boundary_terms.size() << " boundary term(s)\n";

    // For each boundary term, create a VariationalBC
    unsigned int bc_idx = 0;
    for (const auto & bt : contributions.boundary_terms)
    {
      if (_verbose)
      {
        _console << "\n  Boundary term " << (bc_idx + 1) << ":\n";
        _console << "    Variable: " << bt.variable_name << "\n";
        _console << "    Coefficient: " << bt.coefficient->toString() << "\n";
        _console << "    Field derivative order: " << bt.field_derivative_order << "\n";
        _console << "    Test derivative order: " << bt.test_derivative_order << "\n";
        _console << "    Sign: " << (bt.sign > 0 ? "+" : "-") << "\n";
        _console << "    Description: " << bt.description << "\n";
      }

      // Build the boundary expression: sign * (coefficient · normal)
      // The coefficient is already computed from variational derivative
      // We need to contract it with the normal vector

      std::string bc_name = var_name + "_natural_bc";
      if (bc_idx > 0)
        bc_name += "_" + std::to_string(bc_idx);

      // Create the boundary expression string
      // Format: sign * dot(coefficient, n)
      std::string coeff_str = bt.coefficient->toString();
      std::string sign_str = (bt.sign > 0) ? "" : "-";
      std::string boundary_expr = sign_str + "dot(" + coeff_str + ", n)";

      if (_verbose)
        _console << "    Generated boundary expression: " << boundary_expr << "\n";

      // Create VariationalBC parameters
      InputParameters params = _factory.getValidParams("VariationalBC");
      params.set<NonlinearVariableName>("variable") = bt.variable_name;

      // Determine which boundaries to apply natural BCs to
      std::vector<BoundaryName> bc_boundaries;

      if (isParamValid("natural_bc_boundaries"))
      {
        // User explicitly specified which boundaries to use
        bc_boundaries = getParam<std::vector<BoundaryName>>("natural_bc_boundaries");

        if (_verbose)
          _console << "    Using user-specified boundaries for natural BC\n";
      }
      else
      {
        // Default: apply to all boundaries
        bc_boundaries = {"all"};

        if (_verbose)
          _console << "    Using default 'all' boundaries for natural BC\n";
      }

      // Remove any excluded boundaries
      if (isParamValid("exclude_bc_boundaries"))
      {
        const auto & excluded = getParam<std::vector<BoundaryName>>("exclude_bc_boundaries");

        if (_verbose && !excluded.empty())
        {
          _console << "    Excluding boundaries: ";
          for (size_t i = 0; i < excluded.size(); ++i)
          {
            if (i > 0) _console << ", ";
            _console << excluded[i];
          }
          _console << "\n";
        }

        // Remove excluded boundaries from the list
        bc_boundaries.erase(
          std::remove_if(bc_boundaries.begin(), bc_boundaries.end(),
            [&excluded](const BoundaryName& bn) {
              return std::find(excluded.begin(), excluded.end(), bn) != excluded.end();
            }),
          bc_boundaries.end());
      }

      // Skip if no boundaries remain after exclusions
      if (bc_boundaries.empty())
      {
        if (_verbose)
          _console << "    No boundaries remaining after exclusions, skipping natural BC\n";
        bc_idx++;
        continue;
      }

      params.set<std::vector<BoundaryName>>("boundary") = bc_boundaries;

      params.set<std::string>("boundary_expression") = boundary_expr;
      params.set<bool>("use_automatic_differentiation") = _use_automatic_differentiation;

      // Pass parameters if any
      if (isParamValid("parameters"))
        params.set<std::map<std::string, Real>>("parameters") =
            getParam<std::map<std::string, Real>>("parameters");

      // Pass coupled variables
      std::vector<VariableName> coupled_vars;
      for (const auto & name : _variable_names)
      {
        if (name != bt.variable_name)
          coupled_vars.push_back(name);
      }
      for (const auto & name : _coupled_variable_names)
      {
        if (name != bt.variable_name &&
            std::find(coupled_vars.begin(), coupled_vars.end(), name) == coupled_vars.end())
          coupled_vars.push_back(name);
      }

      if (!coupled_vars.empty())
        params.set<std::vector<VariableName>>("coupled_variables") = coupled_vars;

      // Add the boundary condition
      _problem->addBoundaryCondition("VariationalBC", bc_name, params);

      if (_verbose)
      {
        _console << "    Added VariationalBC: " << bc_name << " on boundaries: ";
        for (size_t i = 0; i < bc_boundaries.size(); ++i)
        {
          if (i > 0) _console << ", ";
          _console << bc_boundaries[i];
        }
        _console << "\n";
      }

      bc_idx++;
    }
  }

  if (_verbose)
    _console << "\n[AutomaticWeakForm] Natural boundary condition generation complete\n";
}

std::string
AutomaticWeakFormAction::generateKernelName(const std::string & var_name,
                                            const std::string & suffix)
{
  std::string name = var_name + "_variational";
  if (!suffix.empty())
    name += "_" + suffix;
  return name;
}

void
AutomaticWeakFormAction::writeWeakFormToFile()
{
  std::ofstream file(_weak_form_file);

  if (!file.is_open())
  {
    mooseWarning("Could not open weak form output file: ", _weak_form_file);
    return;
  }

  file << "# Automatically generated weak form from strong form PDEs\n";
  file << "# Variables: ";
  for (const auto & var : _variable_names)
    file << var << " ";
  file << "\n\n";

  file << "# Strong form equations:\n";
  for (const auto & [var_name, eq] : _strong_form_equations)
  {
    file << "#   " << var_name;
    if (eq.type == moose::automatic_weak_form::StringExpressionParser::EquationType::TRANSIENT)
      file << "_t";
    file << " = " << eq.rhs->toString() << "\n";
  }
  file << "\n";

  if (!_c0_forms.empty() || !_c1_forms.empty())
  {
    for (const auto & kv : _c0_forms)
    {
      const auto & var_name = kv.first;
      const auto & c0 = kv.second;
      const auto it = _c1_forms.find(var_name);
      const auto & c1 = (it != _c1_forms.end()) ? it->second : nullptr;
      file << "# Weak-form coefficients for variable: " << var_name << "\n";
      using namespace moose::automatic_weak_form;
      auto sc0 = ExpressionSimplifier::simplify(c0);
      NodePtr sc1 = nullptr;
      if (c1)
        sc1 = ExpressionSimplifier::simplify(c1);
      file << "c0: " << (sc0 ? sc0->toString() : std::string("0")) << "\n";
      file << "c1: " << (sc1 ? sc1->toString() : std::string("[0,0,0]")) << "\n";
      // Unified residual: c0*test + c1·grad(test)
      NodePtr unified_residual = nullptr;
      if (sc0)
        unified_residual = multiply(sc0, testFunction(var_name, false));
      if (sc1)
      {
        NodePtr term1 = dot(sc1, testFunction(var_name, true));
        unified_residual = unified_residual ? add(unified_residual, term1) : term1;
      }
      if (unified_residual)
      {
        auto simplified_unified = ExpressionSimplifier::simplify(unified_residual);
        file << "residual: " << simplified_unified->toString() << "\n\n";
      }
      else
        file << "residual: 0\n\n";
    }
  }
  else
  {
    for (const auto & [var_name, weak_form] : _weak_forms)
    {
      file << "# Weak form for variable: " << var_name << "\n";
      file << (weak_form ? weak_form->toString() : std::string("<null>")) << "\n\n";
    }
  }

  if (!_split_variables.empty())
  {
    file << "# Split variables:\n";
    for (const auto & [name, sv] : _split_variables)
    {
      file << "# " << sv.toString() << "\n";
    }
  }

  file.close();
}

void
AutomaticWeakFormAction::performConservationCheck()
{
  for (const auto & [var_name, weak_form] : _weak_forms)
  {
    auto check = _weak_form_gen->checkConservation(weak_form, var_name);

    if (check.is_conservative)
      _console << "Variable " << var_name << " is conservative\n";
    else
      _console << "Variable " << var_name << " may not be conservative: " << check.message << "\n";
  }
}

void
AutomaticWeakFormAction::performStabilityAnalysis()
{
  auto analysis = _problem_analyzer->analyze(_energy_functional, _variable_names);

  _console << "\nStability Analysis:\n";
  _console << "  Well-posed: " << (analysis.is_well_posed ? "Yes" : "No") << "\n";
  _console << "  Elliptic: " << (analysis.is_elliptic ? "Yes" : "No") << "\n";
  _console << "  Parabolic: " << (analysis.is_parabolic ? "Yes" : "No") << "\n";
  _console << "  Required continuity: C^" << analysis.required_continuity << "\n";
  _console << "  Recommended FE order: " << analysis.recommended_fe_order << "\n";

  if (!analysis.warnings.empty())
  {
    _console << "  Warnings:\n";
    for (const auto & warning : analysis.warnings)
      _console << "    - " << warning << "\n";
  }

  if (!analysis.suggestions.empty())
  {
    _console << "  Suggestions:\n";
    for (const auto & suggestion : analysis.suggestions)
      _console << "    - " << suggestion << "\n";
  }
}

void
AutomaticWeakFormAction::parseStrongForms()
{
  // Create parser
  moose::automatic_weak_form::StringExpressionParser parser(_problem->mesh().dimension());

  // Enable verbose mode if requested
  parser.setVerbose(_verbose);

  // Set up parameters
  if (isParamValid("parameters"))
  {
    const auto & params = getParam<std::map<std::string, Real>>("parameters");
    for (const auto & [name, value] : params)
      parser.setParameter(name, value);
  }

  // Define all variables
  for (const auto & var_name : _variable_names)
    parser.defineVariable(var_name);
  for (const auto & var_name : _coupled_variable_names)
    parser.defineVariable(var_name);

  // Parse intermediate expressions if provided
  if (isParamValid("expressions"))
  {
    const std::string & expr_string = getParam<std::string>("expressions");

    // Split by semicolons
    std::vector<std::string> expressions;
    MooseUtils::tokenize(expr_string, expressions, 1, ";");

    for (const auto & expr : expressions)
    {
      // Skip empty expressions
      if (expr.empty())
        continue;

      // Each expression should be of the form "name = expression"
      size_t eq_pos = expr.find('=');
      if (eq_pos != std::string::npos)
      {
        std::string name = expr.substr(0, eq_pos);
        std::string rhs = expr.substr(eq_pos + 1);

        // Trim whitespace
        name.erase(0, name.find_first_not_of(" \t\n"));
        name.erase(name.find_last_not_of(" \t\n") + 1);
        rhs.erase(0, rhs.find_first_not_of(" \t\n"));
        rhs.erase(rhs.find_last_not_of(" \t\n") + 1);

        // Skip if either name or rhs is empty after trimming
        if (name.empty() || rhs.empty())
          continue;

        parser.defineExpression(name, rhs);
      }
    }
  }

  // Parse strong form equations
  if (isParamValid("strong_forms"))
  {
    const std::string & forms_string = getParam<std::string>("strong_forms");

    // The string contains semicolon-separated equations
    _strong_form_equations = parser.parseStrongForms(forms_string);

    if (_verbose)
    {
      _console << "\n[AutomaticWeakForm] Parsed strong form equations:" << std::endl;
      for (const auto & kv : _strong_form_equations)
      {
        const auto & var_name = kv.first;
        const auto & eq = kv.second;
        _console << "  - " << var_name
                 << (eq.type == moose::automatic_weak_form::StringExpressionParser::EquationType::TRANSIENT ? "_t" : "")
                 << " = " << (eq.rhs ? eq.rhs->toString() : std::string("<null>")) << std::endl;
      }
    }
  }
}

void
AutomaticWeakFormAction::deriveWeakForms()
{
  // For each strong form equation, derive the weak form
  // This involves:
  // 1. Multiplying by test function
  // 2. Integrating by parts if needed
  // 3. Computing Jacobian terms

  // Build an IBP lowerer for current dimension
  moose::automatic_weak_form::StrongFormLowerer lowerer(_problem->mesh().dimension());
  lowerer.setVerbose(_verbose || _debug_print_weak_form);
  if (_verbose)
  {
    // Route lowerer traces through the action console with a prefix
    lowerer.setTracer([this](const std::string & m) { _console << "    [IBP] " << m << "\n"; });
  }

  _c0_forms.clear();
  _c1_forms.clear();

  for (auto & [var_name, eq] : _strong_form_equations)
  {
    // The weak form residual for equation du/dt = F or 0 = F is:
    // R = ∫ test * (F) dx  (after appropriate integration by parts)

    // Apply integration by parts to obtain weak contributions (ignore boundary terms)
    if (_verbose)
    {
      _console << "\n[AutomaticWeakForm] Deriving weak form for variable: " << var_name << std::endl;
      _console << "  RHS (strong): " << (eq.rhs ? eq.rhs->toString() : std::string("<null>")) << std::endl;
      _console << "  Applying IBP lowering..." << std::endl;
    }

    auto lowered = lowerer.lowerToWeak(eq.rhs);
    // Simplify coefficients for cleaner output and evaluation
    auto sc0 = moose::automatic_weak_form::ExpressionSimplifier::simplify(lowered.c0);
    auto sc1 = moose::automatic_weak_form::ExpressionSimplifier::simplify(lowered.c1);

    // If the vector coefficient simplifies to a scalar zero, drop it entirely
    if (sc1 && sc1->type() == moose::automatic_weak_form::NodeType::Constant)
    {
      auto cnode = std::static_pointer_cast<moose::automatic_weak_form::ConstantNode>(sc1);
      if (cnode->value().isScalar() && std::abs(cnode->value().asScalar()) == 0.0)
        sc1 = nullptr;
    }

    _c0_forms[var_name] = sc0;
    _c1_forms[var_name] = sc1;

    // Optional: print Jacobian derivative terms (symbolic) for debugging
    if (_debug_print_jacobian)
    {
      std::vector<std::string> all_vars;
      for (const auto & n : _variable_names)
        if (std::find(all_vars.begin(), all_vars.end(), n) == all_vars.end())
          all_vars.push_back(n);
      for (const auto & n : _coupled_variable_names)
        if (std::find(all_vars.begin(), all_vars.end(), n) == all_vars.end())
          all_vars.push_back(n);

      _console << "  Jacobian terms (symbolic):" << std::endl;
      for (const auto & wrt : all_vars)
      {
        moose::automatic_weak_form::DifferentiationVisitor dv(wrt);

        if (lowered.c0)
        {
          auto d0 = dv.differentiate(lowered.c0);
          auto d0_u = d0.getCoefficient(0);
          auto d0_grad = d0.getCoefficient(1);
          _console << "    d(c0)/d(" << wrt << ") = "
                   << (d0_u ? d0_u->toString() : std::string("0")) << std::endl;
          _console << "    d(c0)/d(grad(" << wrt << ")) = "
                   << (d0_grad ? d0_grad->toString() : std::string("0")) << std::endl;
        }

        if (lowered.c1)
        {
          auto d1 = dv.differentiate(lowered.c1);
          auto d1_u = d1.getCoefficient(0);
          auto d1_grad = d1.getCoefficient(1);
          _console << "    d(c1)/d(" << wrt << ") = "
                   << (d1_u ? d1_u->toString() : std::string("0")) << std::endl;
          _console << "    d(c1)/d(grad(" << wrt << ")) = "
                   << (d1_grad ? d1_grad->toString() : std::string("0")) << std::endl;
        }
      }
    }

    if (_verbose || _debug_print_weak_form)
    {
      _console << "  Resulting weak-form coefficients (simplified):" << std::endl;
      _console << "    c0 (multiplies test): "
               << (_c0_forms[var_name] ? _c0_forms[var_name]->toString() : std::string("0")) << std::endl;
      _console << "    c1 (multiplies grad(test)): "
               << (_c1_forms[var_name] ? _c1_forms[var_name]->toString() : std::string("[0,0,0]")) << std::endl;

      // Unified residual AST: c0*test + c1·grad(test)
      using namespace moose::automatic_weak_form;
      NodePtr unified_residual = nullptr;
      if (_c0_forms[var_name])
        unified_residual = multiply(_c0_forms[var_name], testFunction(var_name, false));
      if (_c1_forms[var_name])
      {
        NodePtr term1 = dot(_c1_forms[var_name], testFunction(var_name, true));
        unified_residual = unified_residual ? add(unified_residual, term1) : term1;
      }
      if (unified_residual)
      {
        auto simplified_unified = ExpressionSimplifier::simplify(unified_residual);
        _console << "    Unified residual (-F): " << simplified_unified->toString() << std::endl;
        // Also print the IBP of +F for clarity: F*test = - (c0*test + c1·grad(test))
        NodePtr f_unified = nullptr;
        if (_c0_forms[var_name])
          f_unified = multiply(negate(_c0_forms[var_name]), testFunction(var_name, false));
        if (_c1_forms[var_name])
        {
          NodePtr t1 = dot(negate(_c1_forms[var_name]), testFunction(var_name, true));
          f_unified = f_unified ? add(f_unified, t1) : t1;
        }
        if (f_unified)
        {
          auto simplified_f = ExpressionSimplifier::simplify(f_unified);
          _console << "    Unified RHS (+F): " << simplified_f->toString() << std::endl;
        }
      }
    }

    // Keep original RHS as a placeholder/debug weak_residual
    eq.weak_residual = eq.rhs;

    // TODO: Compute Jacobian symbolically or mark for AD
    eq.jacobian = nullptr;
  }
}

void
AutomaticWeakFormAction::addTimeDerivativeKernels()
{
  // Add TimeDerivative kernels for transient terms
  for (const auto & [var_name, eq] : _strong_form_equations)
  {
    if (eq.type == moose::automatic_weak_form::StringExpressionParser::EquationType::TRANSIENT)
    {
      std::string kernel_name = var_name + "_time_derivative";

      InputParameters params = _factory.getValidParams("TimeDerivative");
      params.set<NonlinearVariableName>("variable") = var_name;

      _problem->addKernel("TimeDerivative", kernel_name, params);
    }
  }
}

void
AutomaticWeakFormAction::addExpressionKernels()
{
  if (_verbose)
    _console << "\n[AutomaticWeakForm] Adding kernels for strong form equations\n";

  // Add VariationalKernelBase for each equation's RHS
  for (const auto & [var_name, eq] : _strong_form_equations)
  {
    std::string kernel_name = var_name + "_strong_form_kernel";

    // Always use lowered weak form via kernel for strong_forms
    const bool have_lowered = _c0_forms.count(var_name);
    std::string kernel_type = _use_automatic_differentiation ? "ADVariationalKernelBase" : "VariationalKernelBase";

    if (_verbose)
    {
      _console << "\n  Variable: " << var_name << "\n";
      _console << "    Type: " << (eq.type == moose::automatic_weak_form::StringExpressionParser::EquationType::TRANSIENT ? "TRANSIENT" : "STEADY") << "\n";
      _console << "    Kernel type: " << kernel_type << "\n";
      _console << "    RHS expression: " << eq.rhs->toString() << "\n";
      if (have_lowered)
      {
        _console << "    Lowered weak form (c0): " << _c0_forms.at(var_name)->toString() << "\n";
        const auto it1 = _c1_forms.find(var_name);
        auto c1 = (it1 != _c1_forms.end()) ? it1->second : nullptr;
        _console << "    Lowered weak form (c1): "
                 << (c1 ? c1->toString() : std::string("[none]")) << "\n";
      }
    }

    InputParameters params = _factory.getValidParams(kernel_type);
    params.set<NonlinearVariableName>("variable") = var_name;

    params.set<bool>("use_lowered_weak_form") = true;
    params.set<std::string>("c0_expression") = _c0_forms[var_name] ? _c0_forms[var_name]->toString() : std::string("0.0");
    {
      const auto itc1 = _c1_forms.find(var_name);
      auto c1 = (itc1 != _c1_forms.end()) ? itc1->second : nullptr;
      params.set<std::string>("c1_expression") = c1 ? c1->toString() : std::string("");
    }

    // Respect AD selection; AD kernel will drive Jacobian via dual numbers
    params.set<bool>("use_automatic_differentiation") = _use_automatic_differentiation;
    params.set<bool>("compute_jacobian_numerically") = _compute_jacobian_numerically;
    params.set<Real>("fd_eps") = _fd_epsilon;

    // Pass splitting configuration
    params.set<bool>("enable_variable_splitting") = _enable_splitting;
    params.set<unsigned int>("fe_order") = _max_fe_order;

    // Mark as strong form (expression is residual, not energy functional)
    params.set<bool>("is_strong_form") = true;

    // Pass parameters if provided
    if (isParamValid("parameters"))
    {
      const auto & param_map = getParam<std::map<std::string, Real>>("parameters");
      params.set<std::map<std::string, Real>>("parameters") = param_map;

      if (_verbose && !param_map.empty())
      {
        _console << "    Parameters: ";
        bool first = true;
        for (const auto & [pname, pvalue] : param_map)
        {
          if (!first) _console << ", ";
          _console << pname << "=" << pvalue;
          first = false;
        }
        _console << "\n";
      }
    }

    // Collect coupled variables (all other variables in the system)
    std::vector<VariableName> coupled_vars;
    for (const auto & name : _variable_names)
    {
      if (name != var_name)
        coupled_vars.push_back(name);
    }
    for (const auto & name : _coupled_variable_names)
    {
      if (name != var_name && std::find(coupled_vars.begin(), coupled_vars.end(), name) == coupled_vars.end())
        coupled_vars.push_back(name);
    }

    if (!coupled_vars.empty())
    {
      params.set<std::vector<VariableName>>("coupled_vars") = coupled_vars;

      if (_verbose)
      {
        _console << "    Coupled variables: ";
        for (size_t i = 0; i < coupled_vars.size(); ++i)
        {
          if (i > 0) _console << ", ";
          _console << coupled_vars[i];
        }
        _console << "\n";
      }
    }

    _problem->addKernel(kernel_type, kernel_name, params);

    if (_verbose)
      _console << "    ✓ Kernel added successfully\n";
  }
}
