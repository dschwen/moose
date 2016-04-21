[Mesh]
  file = plainmesh.e
[]

[Functions]
  [./init]
    type = ParsedFunction
    value = 'if(x < -0.3 & y < -0.3, 1.0, 0.0)'
  [../]
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = init
    [../]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = c
  [../]
  [./dt]
    type = TimeDerivative
    variable = c
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON

  nl_rel_tol = 1e-11
  nl_abs_tol = 1e-10
  l_tol = 1e-10
  dt = 0.02
  num_steps = 20
[]

[Outputs]
  exodus = true
[]
