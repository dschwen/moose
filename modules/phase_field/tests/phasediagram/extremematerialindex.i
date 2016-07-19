[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[AuxVariables]
  [./index]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./vx]
    [./InitialCondition]
      type = FunctionIC
      function = x
    [../]
  [../]
  [./vy]
    [./InitialCondition]
      type = FunctionIC
      function = y
    [../]
  [../]
[]

[AuxKernels]
  [./index]
    type = ExtremeMaterialIndexAux
    variable = index
    props = 'Fa Fb Fc'
    execute_on = timestep_end
  [../]
[]

[Materials]
  [./Fa]
    type = ParsedMaterial
    args = 'vx vy'
    f_name = Fa
    function = 'sin(vx*4)*cos(vy*3)'
    outputs = exodus
    output_properties = Fa
  [../]

  [./Fb]
    type = ParsedMaterial
    args = 'vx vy'
    f_name = Fb
    function = 'sin(vx*3)*cos(vy*2)'
    outputs = exodus
    output_properties = Fb
  [../]

  [./Fc]
    type = ParsedMaterial
    args = 'vx vy'
    f_name = Fc
    function = 'cos(vx*2)*cos(vy*5)'
    outputs = exodus
    output_properties = Fc
  [../]
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
  execute_on = final
[]
