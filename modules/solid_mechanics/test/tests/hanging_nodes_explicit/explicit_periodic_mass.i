[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 4
  xmin = 0.0
  xmax = 1.0
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
  # Add a tiny reaction term to avoid a purely free system
  [./react]
    type = Reaction
    variable = u
    coefficient = 0.0
  [../]
[]

[BCs]
  [Periodic]
    [./px]
      variable = u
      auto_direction = 'x'
    [../]
  []
[]

[Executioner]
  type = Transient
  dt = 1e-6
  num_steps = 1
  solve_type = LINEAR

  [./TimeIntegrator]
    type = ExplicitSSPRungeKutta
    order = 1
    solve_type = lumped
  [../]
[]

[Outputs]
  csv = true
  execute_on = 'final'
[]

