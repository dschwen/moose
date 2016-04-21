[Mesh]
  file = mortarmesh.e

  [./MortarInterfaces]
    [./up_down]
      master = 1
      slave = 2
      subdomain = 2
    [../]
    [./left_right]
      master = 3
      slave = 4
      subdomain = 3
    [../]
  [../]
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
    block = 1
    [./InitialCondition]
      type = FunctionIC
      function = init
    [../]
  [../]

  [./lm_up_down]
    order = FIRST
    family = LAGRANGE
    block = 2
  [../]
  [./lm_left_right]
    order = FIRST
    family = LAGRANGE
    block = 3
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

[Constraints]
  [./equal1]
    type = EqualValueConstraint
    variable = lm_up_down
    interface = up_down
    master_variable = c
  [../]
  [./equal2]
    type = EqualValueConstraint
    variable = lm_left_right
    interface = left_right
    master_variable = c
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
