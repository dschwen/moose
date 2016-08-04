[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 2
[]

[Variables]
  [./c]
    #initial_condition = 0
    [./InitialCondition]
      type = FunctionIC
      function = 1-x
    [../]
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = c
    value = 1.0
    boundary = left
  [../]
[]

[Kernels]
  [./c]
    type = StabilizeGradient
    variable = c
    direction = '1 0 0'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  line_search = none
  nl_max_its = 50
  l_max_its = 20
[]

[Outputs]
  exodus = true
[]
