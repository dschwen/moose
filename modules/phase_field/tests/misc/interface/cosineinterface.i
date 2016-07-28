[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = -3
  xmax = 7
  nx = 50
[]

[Variables]
  [./eta]
  [../]
[]

[Kernels]
  [./eta]
    type = CosineInterface
    variable = eta
  [../]
[]

[Preconditioning]
  [./fdp]
    type = FDP
    full = true
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = eta
    boundary = left
    value = 1.0
  [../]
  [./right]
    type = DirichletBC
    variable = eta
    boundary = right
    value = 0.0
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  line_search = none

  #petsc_options_iname = '-pc_type'
  #petsc_options_value = 'lu'
  petsc_options = '-snes_converged_reason'
[]

[Outputs]
  exodus = true
[]
