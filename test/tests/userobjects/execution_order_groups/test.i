[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
[]

[AuxVariables]
  [v]
    initial_condition = 1
  []
[]

[Postprocessors]
  [pp1]
    type = ExecutionGroupTestPostprocessor
    variable = v
    execute_on = INITIAL
  []
  [pp2]
    type = ExecutionGroupTestPostprocessor
    variable = v
    execute_on = INITIAL
    chained_postprocessor = pp1
  []
  [pp3]
    type = ExecutionGroupTestPostprocessor
    variable = v
    execute_on = INITIAL
    chained_postprocessor = pp2
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
