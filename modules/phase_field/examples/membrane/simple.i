[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
[]

[Variables]
  [./h]
    [./InitialCondition]
      type = RandomIC
      min = -0.1
      max = 0.1
    [../]
  [../]
  [./u]
  [../]
[]

[Kernels]
  [./curvature]
    type = LaplacianSplit
    variable = u
    c = h
  [../]
  [./bend]
    type = BodyForce
    variable = u
    value = 0.1
  [../]

  [./dt]
    type = TimeDerivative
    variable = h
  [../]
  [./ac]
    type = LaplacianSplit
    variable = h
    c = u
  [../]
[]

[Preconditioning]
  [./smp]
    #type = SMP
    type = FDP
    full = true
  [../]
[]

[BCs]
  [./clamp]
    type = DirichletBC
    variable = h
    boundary = 'top bottom left right'
    value = 0
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  line_search = none
  petsc_options_iname = '-pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'lu       nonzero'
  dt = 1

  nl_rel_tol = 1e-08
  nl_abs_tol = 1e-10
  l_max_its = 100

  num_steps = 10
[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
