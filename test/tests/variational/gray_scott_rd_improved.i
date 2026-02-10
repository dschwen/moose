# Gray-Scott Reaction-Diffusion Model - IMPROVED TIMESTEPING
#
# Improvements over gray_scott_rd.i:
# 1. Relaxed tolerances appropriate for pattern formation
# 2. More aggressive adaptive timestepping
# 3. Automatic scaling enabled
# 4. Better preconditioner for larger timesteps
# 5. Larger initial timestep

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
  # u = 1.0 + small random noise to break symmetry
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

[AutomaticWeakForm]
  strong_forms = 'u_t = Du*laplacian(u) - u*v*v + F*(1-u);
                  v_t = Dv*laplacian(v) + u*v*v - (F+k)*v'

  variables = 'u v'
  parameters = 'Du 2e-5 Dv 5e-6 F 0.04 k 0.06'

  use_automatic_differentiation = false
  enable_splitting = false
  verbose = false  # Turn off for faster running
[]

[BCs]
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
    dt = 0.5
    optimal_iterations = 12
    iteration_window = 2
    growth_factor = 2.0     # More aggressive growth
    cutback_factor = 0.5    # More aggressive cutback when needed
  []

  dtmin = 0.01   # Increased from 0.001
  dtmax = 50.0   # Add maximum to prevent excessive growth

  end_time = 1000

  # RELAXED tolerances - appropriate for pattern formation
  nl_rel_tol = 1e-6   # Relaxed from 1e-8
  nl_abs_tol = 1e-8   # Relaxed from 1e-10
  nl_max_its = 15     # Allow a few more iterations

  l_tol = 1e-4        # Linear tolerance
  l_max_its = 50      # Linear iterations

  # Better preconditioning for reaction-diffusion
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre    boomeramg'

  # Enable automatic scaling to handle scale mismatch
  automatic_scaling = true
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
  [dt]
    type = TimestepSize
  []
[]
