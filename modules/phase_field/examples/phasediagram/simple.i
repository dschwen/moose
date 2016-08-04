[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20

  # concentration exis
  xmin = 0.001
  xmax = 0.999

  # temperature in eV
  ymin = 0.01
  ymax = 0.6
[]

[AuxVariables]
  # concentration
  [./c]
    order = CONSTANT
    family = MONOMIAL
  [../]
  # Temperature
  [./T]
    order = CONSTANT
    family = MONOMIAL
  [../]

  # resulting phase ID
  [./phase]
    order = CONSTANT
    family = MONOMIAL
  [../]

  [./xb]
    initial_condition = 0.9
  [../]
[]

[AuxKernels]
  # use FunctionAux kernels to allow for refinement
  [./c]
    type = FunctionAux
    variable = c
    function = x
  [../]
  [./T]
    type = FunctionAux
    variable = T
    function = y
  [../]

  [./phase]
    type = ExtremeMaterialIndexAux
    variable = phase
    value_type = min
    props = 'Fa Fb Fgap_mask'
  [../]
[]

[Variables]
  # terminal concentrations for the miscibility gap
  [./xa]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = 0.01
  [../]
[]

[Kernels]
  [./xa]
    type = MinimizeProperty
    variable = xa
    prop = Fgap
    args = xb
  [../]
[]

[Materials]
  [./Fa]
    type = DerivativeParsedMaterial
    f_name = Fa
    args = 'c T'
    function = '(c-0.1)^2'
    outputs = exodus
    output_properties = Fa
  [../]
  [./Fb]
    type = DerivativeParsedMaterial
    f_name = Fb
    args = 'c T'
    function = '100*(0.9-c)^2'
    outputs = exodus
    output_properties = Fb
  [../]

  [./Fxa]
    type = DerivativeParsedMaterial
    f_name = Fxa
    args = 'xa T'
    function = '(xa-0.1)^2'
    outputs = exodus
    output_properties = Fxa
  [../]
  [./Fxb]
    type = DerivativeParsedMaterial
    f_name = Fxb
    args = 'xb T'
    function = '100*(0.9-xb)^2'
    outputs = exodus
    output_properties = Fxb
  [../]

  [./Fgap]
    type = DerivativeParsedMaterial
    f_name = Fgap
    material_property_names = 'Fxa(xa) Fxb(xb)'
    function = '1/(xb-xa) * ((xb-c)*Fxa + (c-xa)*Fxb)'
    args = 'xa xb c'
    outputs = exodus
    output_properties = Fgap
  [../]

  [./Fgap_mask]
    type = ParsedMaterial
    f_name = Fgap_mask
    material_property_names = 'Fxa(xa) Fxb(xb)'
    function = '1/(xb-xa) * ((xb-c)*Fxa + (c-xa)*Fxb) + if(xa<xb,if(c<xa|c>xb,1000,0),if(c<xb|c>xa,1000,0))'
    args = 'xa xb c'
    outputs = exodus
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_max_its = 200
  l_max_its = 50
[]

[Outputs]
  exodus = true
[]
