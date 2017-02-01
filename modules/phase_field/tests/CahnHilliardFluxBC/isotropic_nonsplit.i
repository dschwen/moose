[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 5.0
  ymax = 5.0
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = THIRD
    family = HERMITE
    [./InitialCondition]
      type = RandomIC
      max = 0.1
      min = -0.1
    [../]
  [../]
  [./d]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1
  [../]
[]

[Kernels]
  [./cres]
    type = CahnHilliard
    variable = c
    f_name = F
    mob_name = M
  [../]
  [./int]
    type = CHInterface
    variable = c
    mob_name = M
    kappa_name = kappa_c
  [../]
  [./time]
    type = TimeDerivative
    variable = c
  [../]

  [./diff]
    type = MatDiffusion
    D_name = 10.0
    variable = d
  [../]
  [./time2]
    type = TimeDerivative
    variable = d
  [../]
[]

[BCs]
  [./in_flux]
    type = CahnHilliardFluxBC
    variable = c
    boundary = top
    flux = '0 100.0 0'
    mob_name = M
    args = 'c d'
    penalty = 1000
  [../]
  [./out_flux]
    type = CahnHilliardFluxBC
    variable = c
    boundary = bottom
    flux = '0 0.0 0'
    mob_name = M
    args = 'c d'
    penalty = 1000
  [../]
  [./Periodic]
    [./c]
      auto_direction = x
      variable = c
    [../]
  [../]

  [./dirichlet_left]
    type = DirichletBC
    boundary = left
    variable = d
    value = 0
  [../]
  [./dirichlet_right]
    type = DirichletBC
    boundary = right
    variable = d
    value = 2
  [../]
[]

[Materials]
  [./constant]
    type = GenericConstantMaterial
    prop_names  = 'kappa_c'
    prop_values = '2.0'
  [../]
  [./mob]
    type = DerivativeParsedMaterial
    f_name = M
    function = 'c^2+d+0.1'
    args = 'c d'
  [../]
  [./F]
    type = DerivativeParsedMaterial
    f_name = F
    function = 'c^2'
    args = 'c'
  [../]
[]

[Postprocessors]
  [./total_solute]
    type = ElementIntegralVariablePostprocessor
    variable = c
    execute_on = 'timestep_end'
    outputs = none
  [../]
  [./initial_solute]
    type = ElementIntegralVariablePostprocessor
    variable = c
    execute_on = 'initial'
    outputs = none
  [../]
  [./solute_change]
    type = DifferencePostprocessor
    value1 = total_solute
    value2 = initial_solute
    execute_on = 'timestep_end'
  [../]
[]
#
#[Preconditioning]
#  [./SMP]
#    type = SMP
#    full = true
#  [../]
#[]

[Executioner]
  type = Transient
  scheme = 'BDF2'
  solve_type = 'PJFNK'
  line_search = none

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 31'

  l_max_its = 30
  l_tol = 1.0e-3
  nl_max_its = 10
  nl_rel_tol = 1.0e-11
  nl_abs_tol = 1.0e-10
  num_steps = 4

  dt = 1
[]

[Outputs]
  csv = true
  print_linear_residuals = false
  print_perf_log = true
[]
