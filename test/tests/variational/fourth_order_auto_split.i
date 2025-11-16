# Fourth-order automatic splitting validation test
# This test validates that the automatic splitting system can handle
# a fourth-order energy functional and produce the same results as
# a hand-crafted split configuration.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0.0
  xmax = 1.0
[]

[Variables]
  [c]
    family = LAGRANGE
    order = FIRST
  []
[]

[ICs]
  [c_ic]
    type = FunctionIC
    variable = c
    function = '0.1 * sin(2 * pi * x)'
  []
[]

[AutomaticWeakForm]
  # Fourth-order energy with automatic splitting enabled
  strong_forms = 'c = var_diff(W(c) + 0.5*kappa*dot(grad(c), grad(c)) + 0.5*lambda*dot(grad(grad(c)), grad(grad(c))), c)'
  parameters = 'kappa 0.01 lambda 0.001'
  variables = 'c'

  # Enable automatic splitting - this should create split variables
  enable_splitting = true
  max_fe_order = 1  # Force splitting of second-order terms

  use_automatic_differentiation = false
  verbose = true  # Show what split variables are created
[]

[BCs]
  [c_left]
    type = DirichletBC
    variable = c
    boundary = left
    value = 0.0
  []
  [c_right]
    type = DirichletBC
    variable = c
    boundary = right
    value = 0.0
  []

  # Split variables will need their own BCs - these should be added automatically
  # or we need to specify them based on the constraint relationships
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu mumps'

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  nl_max_its = 10

  # Use a smaller initial residual tolerance since we have additional constraint equations
  nl_forced_its = 1
[]

[Outputs]
  exodus = true
  csv = true
  console = true

  [debug]
    type = Console
    execute_on = 'initial nonlinear'  # Show debug info during solve
  []
[]

[Postprocessors]
  [total_energy]
    type = ElementIntegralVariablePostprocessor
    variable = c
    # This should integrate the full energy including split terms
  []

  [c_l2_norm]
    type = ElementL2Norm
    variable = c
  []

  # Check if split variables were created
  [num_dofs]
    type = NumDOFs
  []

  [num_vars]
    type = NumVars
  []
[]
