# Gray-Scott Reaction-Diffusion Model using Automatic Weak Form
#
# This example demonstrates the Gray-Scott reaction-diffusion system
# using the automatic weak form generation from strong form PDEs.
#
# Model equations (strong form):
#   ∂u/∂t = Du·∇²u - u·v² + F·(1-u)
#   ∂v/∂t = Dv·∇²v + u·v² - (F+k)·v
#
# where:
#   u, v = concentrations of two chemical species
#   Du, Dv = diffusion coefficients
#   F = feed rate (replenishment of u)
#   k = kill rate (removal of v)
#
# This configuration uses "mitosis" parameters (F=0.0367, k=0.0649)
# which produce cell-division-like patterns.
#
# Reference: https://www.karlsims.com/rd.html

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 200
  ny = 200
  xmin = 0
  xmax = 2.5
  ymin = 0
  ymax = 2.5
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
  [v]
    order = FIRST
    family = LAGRANGE
  []
[]

[ICs]
  # Start with uniform u=1, v=0 everywhere
  [u_ic]
    type = ConstantIC
    variable = u
    value = 1.0
  []

  # Add a small circular perturbation of v in the center
  # This seeds the pattern formation
  [v_ic]
    type = FunctionIC
    variable = v
    function = 'r := sqrt((x-1.25)^2 + (y-1.25)^2); if(r < 0.1, 1.0, 0.0)'
  []
[]

[AutomaticWeakForm]
  # Use strong form specification
  energy_type = expression

  # Gray-Scott equations in strong form
  # Note: The system automatically handles time derivatives and spatial operators
  strong_forms = 'u_t = Du*laplacian(u) - u*v*v + F*(1-u);
                  v_t = Dv*laplacian(v) + u*v*v - (F+k)*v'

  # Define the variables
  variables = 'u v'

  # Define parameters
  parameters = 'Du 2e-5
                Dv 1e-5
                F 0.0367
                k 0.0649'

  # Enable automatic differentiation for Jacobian
  use_automatic_differentiation = true

  # Enable verbose output to see what's happening
  verbose = true
  debug_print_weak_form = false
[]

[BCs]
  # Periodic boundary conditions - patterns wrap around
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

  # Time stepping
  dt = 1.0
  end_time = 10000

  # Solver tolerances
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  l_tol = 1e-6
  l_max_its = 30

  # Petsc options for efficiency
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 101'

  # Time stepper
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1.0
    optimal_iterations = 6
    iteration_window = 2
    growth_factor = 1.2
    cutback_factor = 0.8
  []
[]

[Outputs]
  print_linear_residuals = false

  [exodus]
    type = Exodus
    execute_on = 'INITIAL TIMESTEP_END'
    interval = 10  # Output every 10 time steps
  []

  [console]
    type = Console
    max_rows = 10
  []
[]

[Postprocessors]
  [u_average]
    type = ElementAverageValue
    variable = u
  []
  [v_average]
    type = ElementAverageValue
    variable = v
  []
  [u_max]
    type = ElementExtremeValue
    variable = u
    value_type = max
  []
  [v_max]
    type = ElementExtremeValue
    variable = v
    value_type = max
  []
  [num_nonlinear_iterations]
    type = NumNonlinearIterations
  []
  [num_linear_iterations]
    type = NumLinearIterations
  []
[]
