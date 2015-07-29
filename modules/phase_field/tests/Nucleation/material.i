[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 50
  nz = 0
  xmin = 0
  xmax = 20
  ymin = 0
  ymax = 20
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.0
  [../]
[]

[Kernels]
  [./c]
    type = Diffusion
    variable = c
  [../]
[]

[Materials]
  [./nucleation]
    type = DiscreteNucleation
    op_names  = c
    op_values = 1
    block = 0
    hold_time = 1
    probability = 0.02
    outputs = exodus
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  num_steps = 30
  dt = 0.1
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
[]
