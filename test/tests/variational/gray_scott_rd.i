# Gray-Scott Reaction-Diffusion Model using Automatic Weak Form
#
# This example demonstrates the Gray-Scott reaction-diffusion system using
# the automatic weak form system with strong form equations.
#
# Model equations (strong form):
#   ∂u/∂t = Du·∇²u - u·v² + F·(1-u)
#   ∂v/∂t = Dv·∇²v + u·v² - (F+k)·v
#
# The AutomaticWeakForm action automatically:
# 1. Parses the strong form equations
# 2. Generates TimeDerivative kernels for ∂u/∂t and ∂v/∂t
# 3. Creates VariationalKernelBase kernels for the RHS terms
# 4. Handles coupling between u and v automatically
#
# Parameters: "mitosis" pattern (F=0.0367, k=0.0649)
# Reference: https://www.karlsims.com/rd.html

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 128
  ny = 128
  xmin = 0
  xmax = 1.25
  ymin = 0
  ymax = 1.25
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[ICs]
  # u = 1.0 + small random noise to break symmetry (match traditional)
  [u]
    type = RandomIC
    variable = u
    min = 0.998
    max = 1.000
    seed = 2342
  []

  [v]
    type = FunctionIC
    variable = v
    function = 'r:=sqrt((x-${Mesh/xmax}/2)^2+(y-${Mesh/ymax}/2)^2); if(r<${Mesh/xmax}/16,1,0)'
  []
[]

# Use AutomaticWeakForm with strong form equations
[AutomaticWeakForm]
  # Strong form equations: u_t = RHS, v_t = RHS
  # Note: laplacian(u) computes ∇²u
  strong_forms = 'u_t = Du*laplacian(u) - u*v*v + F*(1-u);
                  v_t = Dv*laplacian(v) + u*v*v - (F+k)*v'

  variables = 'u v'
  # PDE-scaled parameters for stable patterning on this domain
  parameters = 'Du 2e-5 Dv 5e-6 F 0.04 k 0.06'

  # Coral growth alternative: F=0.0545, k=0.062
  use_automatic_differentiation = false
  enable_splitting = false

  # Enable verbose output to see the automatic weak form generation
  verbose = true
[]

[BCs]
  # Periodic boundary conditions
  [Periodic]
    [all]
      variable = 'u v'
      auto_direction = 'x y'
    []
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.02
    optimal_iterations = 8
    iteration_window = 2
    growth_factor = 1.5
    cutback_factor = 0.75
  []
  dtmin = 0.001
  dtmax = 10

  end_time = 1000
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  nl_max_its = 12

  # Scale diffusion coefficients directly in PETSc options
  # since we're using the standard Diffusion kernel
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
  print_linear_residuals = false

  [console]
    type = Console
    max_rows = 10
  []
[]

[Postprocessors]
  [u_min]
    type = ElementExtremeValue
    value_type = min
    variable = u
  []
  [u_max]
    type = ElementExtremeValue
    value_type = max
    variable = u
  []
  [v_min]
    type = ElementExtremeValue
    value_type = min
    variable = v
  []
  [v_max]
    type = ElementExtremeValue
    value_type = max
    variable = v
  []
[]
