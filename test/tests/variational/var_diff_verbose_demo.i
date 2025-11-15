# Demonstration of verbose output features
# Shows all the debugging information available for var_diff() usage

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 10
[]

[Variables]
  [T]
  []
[]

[ICs]
  [T_ic]
    type = ConstantIC
    variable = T
    value = 1.0
  []
[]

[AutomaticWeakForm]
  energy_type = expression
  # Using var_diff to automatically derive heat equation from energy functional
  energy_expression = 'var_diff(0.5*k*dot(grad(T), grad(T)), T)'
  parameters = 'k 1.0'
  variables = 'T'
  use_automatic_differentiation = true

  # Enable all verbose output options
  verbose = true                      # General verbose output
  debug_print_expressions = true      # Show parsed expressions and AST
  debug_print_derivatives = true      # Show derivative calculations step by step
  debug_print_simplification = true   # Show expression simplification steps
  debug_print_weak_form = true        # Show final weak form expressions
  debug_print_jacobian = true         # Show Jacobian expressions
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = false
  console = true
[]
