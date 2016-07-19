[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[AuxVariables]
  [./a]
    [./InitialCondition]
      type = FunctionIC
      function = 2*x
    [../]
  [../]
  [./b]
    [./InitialCondition]
      type = FunctionIC
      function = y
    [../]
  [../]
[]

[Variables]
  [./c]
  [../]
[]

[Kernels]
  [./c]
    type = MinimizeProperty
    variable = c
    prop = F
    args = c
  [../]
[]

[Materials]
  [./F]
    type = DerivativeParsedMaterial
    args = 'a b c'
    function = '(a + b - c)^2'
    outputs = exodus
    output_properties = F
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
