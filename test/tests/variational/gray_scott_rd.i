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
  nx = 100
  ny = 100
  xmin = 0
  xmax = 20
  ymin = 0
  ymax = 20
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[ICs]
  # Start with u=1 everywhere
  [u_ic]
    type = ConstantIC
    variable = u
    value = 1.0
  []

  # Add a small circular seed of v in the center
  [v_ic]
    type = FunctionIC
    variable = v
    # Karl Sims: seed B (v) with amplitude 1.0 in a small disk
    function = 'r := sqrt((x-${Mesh/xmax}/2)^2 + (y-${Mesh/ymax}/2)^2); tanh((2-r)/1)/2+0.5'
  []
[]

# Use AutomaticWeakForm with strong form equations
[AutomaticWeakForm]
  # Strong form equations: u_t = RHS, v_t = RHS
  # Note: laplacian(u) computes ∇²u
  strong_forms = 'u_t = Du*laplacian(u) - u*v*v + F*(1-u);
                  v_t = Dv*laplacian(v) + u*v*v - (F+k)*v'

  variables = 'u v'
  # Mitosis (Karl Sims): Du=1.0, Dv=0.5, F=0.0367, k=0.0649
  parameters = 'Du 1.0 Dv 0.5 F 0.0367 k 0.0649'
  # Coral growth alternative: F=0.0545, k=0.062
  use_automatic_differentiation = true
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

  # Implicit solve: reasonable starting step for this domain
  dt = 0.5
  end_time = 5000

  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-8
  l_tol = 1e-4
  l_max_its = 30

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.5
    optimal_iterations = 6
    iteration_window = 2
    growth_factor = 1.1
    cutback_factor = 0.5
  []
  dtmin = 0.005
  dtmax = 100.0
[]

[Debug]
  show_actions = true
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
  [u_avg]
    type = ElementAverageValue
    variable = u
  []
  [v_avg]
    type = ElementAverageValue
    variable = v
  []
  [v_max]
    type = ElementExtremeValue
    variable = v
    value_type = max
  []
[]
