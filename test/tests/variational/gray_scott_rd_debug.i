# Gray-Scott Debug Version - Coarse mesh, small time steps to verify correctness
# This should produce smooth patterns if the equations are correct

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 50
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
    function = 'r := sqrt((x-1.25)^2 + (y-1.25)^2); if(r < 0.2, 0.5, 0.0)'
  []
[]

[AutomaticWeakForm]
  strong_forms = 'u_t = Du*laplacian(u) - u*v*v + F*(1-u);
                  v_t = Dv*laplacian(v) + u*v*v - (F+k)*v'

  variables = 'u v'
  parameters = 'Du 2e-5 Dv 1e-5 F 0.0367 k 0.0649'
  use_automatic_differentiation = true
  enable_splitting = false
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
  num_steps = 10

  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-8

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true

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
  [u_max]
    type = ElementExtremeValue
    variable = u
    value_type = max
  []
  [u_min]
    type = ElementExtremeValue
    variable = u
    value_type = min
  []
  [v_max]
    type = ElementExtremeValue
    variable = v
    value_type = max
  []
  [v_min]
    type = ElementExtremeValue
    variable = v
    value_type = min
  []
[]
