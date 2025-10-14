# Fourth-order manual splitting reference test
# This test implements the same physics as fourth_order_auto_split.i
# but using manually defined split variables for comparison.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0.0
  xmax = 1.0
[]

[Variables]
  # Primary variable
  [c]
    family = LAGRANGE
    order = FIRST
  []

  # Split variables (manually defined)
  [c_grad]  # First derivative: c_grad = dc/dx
    family = LAGRANGE
    order = FIRST
  []

  [c_hess]  # Second derivative: c_hess = d²c/dx²
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

  # Initial conditions for split variables based on c
  [c_grad_ic]
    type = FunctionIC
    variable = c_grad
    function = '0.1 * 2 * pi * cos(2 * pi * x)'  # dc/dx
  []

  [c_hess_ic]
    type = FunctionIC
    variable = c_hess
    function = '-0.1 * 4 * pi * pi * sin(2 * pi * x)'  # d²c/dx²
  []
[]

[Kernels]
  # Main variational kernel for primary variable c
  # Residual from δ/δc[W(c) + 0.5*kappa*c_grad² + 0.5*lambda*c_hess²]
  [c_kernel]
    type = VariationalKernelBase
    variable = c
    residual_expression = 'W_prime(c) - kappa * grad_test * c_grad - lambda * hess_test * c_hess'
    coupled = 'c_grad c_hess'
    use_automatic_differentiation = true
    parameters = 'kappa 0.01 lambda 0.001'
  []

  # Constraint kernel for c_grad: c_grad - dc/dx = 0
  [c_grad_constraint]
    type = ScalarGradientConstraint
    variable = c_grad
    u = c
    component = 0  # x-component in 1D
  []

  # Constraint kernel for c_hess: c_hess - d²c/dx² = 0
  [c_hess_constraint]
    type = GradientConstraintKernel
    variable = c_hess
    coupled_gradient = c_grad  # c_hess should equal grad(c_grad)
  []
[]

[BCs]
  # Boundary conditions for primary variable
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

  # Boundary conditions for split variables
  # These are derived from the primary variable BCs
  [c_grad_left]
    type = DirichletBC
    variable = c_grad
    boundary = left
    value = 0.0  # grad(0) = 0
  []
  [c_grad_right]
    type = DirichletBC
    variable = c_grad
    boundary = right
    value = 0.0  # grad(0) = 0
  []

  [c_hess_left]
    type = DirichletBC
    variable = c_hess
    boundary = left
    value = 0.0
  []
  [c_hess_right]
    type = DirichletBC
    variable = c_hess
    boundary = right
    value = 0.0
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu mumps'

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  nl_max_its = 10
[]

[Outputs]
  exodus = true
  csv = true
  console = true
[]

[Postprocessors]
  [total_energy_manual]
    type = ElementIntegralVariablePostprocessor
    variable = c
  []

  [c_l2_norm_manual]
    type = ElementL2Norm
    variable = c
  []

  [num_dofs_manual]
    type = NumDOFs
  []

  [num_vars_manual]
    type = NumVars
  []
[]