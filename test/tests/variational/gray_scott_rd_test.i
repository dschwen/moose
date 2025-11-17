# Gray-Scott Reaction-Diffusion Model - Test Version
# Short run to verify strong form parsing and kernel generation

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = 0
  xmax = 2.5
  ymin = 0
  ymax = 2.5
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[ICs]
  [u_ic]
    type = ConstantIC
    variable = u
    value = 1.0
  []
  [v_ic]
    type = FunctionIC
    variable = v
    function = 'r := sqrt((x-1.25)^2 + (y-1.25)^2); if(r < 0.1, 1.0, 0.0)'
  []
[]

# Use AutomaticWeakForm with strong form equations
[AutomaticWeakForm]
  # Strong form equations: u_t = RHS, v_t = RHS
  strong_forms = 'u_t = Du*laplacian(u) - u*v*v + F*(1-u);
                  v_t = Dv*laplacian(v) + u*v*v - (F+k)*v'

  variables = 'u v'
  parameters = 'Du 2e-5 Dv 1e-5 F 0.0367 k 0.0649'
  use_automatic_differentiation = true
  enable_splitting = false

  # Enable verbose output to see the automatic weak form generation
  verbose = true
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
  dt = 0.1
  end_time = 0.2
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = false
  print_linear_residuals = false

  [console]
    type = Console
    max_rows = 5
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
[]
