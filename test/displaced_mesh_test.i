[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  displacements = 'ux uy'
[]

[AuxVariables]
  [./ux]
    [./InitialCondition]
      type = FunctionIC
      function = x
    [../]
  [../]
  [./uy]
    [./InitialCondition]
      type = FunctionIC
      function = y
    [../]
  [../]
  [./c]
    initial_condition = 1
  [../]
[]

[Variables]
  [./a]
  [../]
[]
[Kernels]
  [./a0]
    type = Diffusion
    variable = a
  [../]
  [./a1]
    type = TimeDerivative
    variable = a
  [../]
[]

[Postprocessors]
  [./without]
    type = ElementIntegralVariablePostprocessor
    variable = c
  [../]
  [./with]
    type = ElementIntegralVariablePostprocessor
    variable = c
    use_displaced_mesh = true
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
