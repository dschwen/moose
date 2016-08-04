[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4

  # concentration exis
  xmin = 0.001
  xmax = 0.999

  # temperature in eV
  ymin = 0.01
  ymax = 0.6

  uniform_refine = 3
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

  # sample point for common tangent
  [./x]
    order = CONSTANT
    family = MONOMIAL
  [../]

  # resulting phase ID
  [./phase]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  # use FunctionAux kernels to allow for refinement
  [./c]
    type = FunctionAux
    variable = c
    function = x
  [../]

  [./x]
    type = ParsedAux
    variable = x
    args = 'xa xb'
    function = '(xa+xb)/2'
    execute_on = 'timestep_begin'
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
    initial_condition = 0.2
  [../]
  [./xb]
    initial_condition = 0.9
  [../]
[]

[Kernels]
  [./xa]
    type = MinimizeProperty
    variable = xa
    prop = Fgap
    args = xb
  [../]
  [./xb]
    type = MinimizeProperty
    variable = xb
    prop = Fgap
    args = xa
  [../]
[]

[Materials]
  [./Fa]
    type = DerivativeParsedMaterial
    f_name = Fa
    args = 'c T'
    constant_names = e
    constant_expressions = 1e-4
    function = 'c^2 + T * (c*plog(c,e) + (1-c)*plog(1-c,e))'
    outputs = exodus
    output_properties = Fa
  [../]
  [./Fb]
    type = DerivativeParsedMaterial
    f_name = Fb
    args = 'c T'
    constant_names = e
    constant_expressions = 1e-4
    function = '0.2 + 10*(0.9-c)^2 + T * (c*plog(c,e) + (1-c)*plog(1-c,e))'
    outputs = exodus
    output_properties = Fb
  [../]

  [./Fxa]
    type = DerivativeParsedMaterial
    f_name = Fxa
    args = 'xa T'
    constant_names = e
    constant_expressions = 1e-4
    function = 'xa^2 + T * (xa*plog(xa,e) + (1-xa)*plog(1-xa,e))'
    outputs = exodus
    output_properties = Fa
  [../]
  [./Fxb]
    type = DerivativeParsedMaterial
    f_name = Fxb
    args = 'xb T'
    constant_names = e
    constant_expressions = 1e-4
    function = '0.5 + 10*(0.9-xb)^2 + T * (xb*plog(xb,e) + (1-xb)*plog(1-xb,e))'
    outputs = exodus
    output_properties = Fb
  [../]

  [./Fgap]
    type = DerivativeParsedMaterial
    f_name = Fgap
    material_property_names = 'Fxa(xa) Fxb(xb)'
    function = '1/(xb-xa) * ((xb-x)*Fxa + (x-xa)*Fxb)'
    args = 'xa xb x'
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

[Adaptivity]
  marker = marker
  steps = 3
  [./Indicators]
    [./indicator]
      type = ValueJumpIndicator
      variable = phase
      outputs = exodus
    [../]
  [../]
  [./Markers]
    [./marker]
      type = ErrorToleranceMarker
      indicator = indicator
      coarsen = 0.00001
      refine = 0.00001
    [../]
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_max_its = 1200
  l_max_its = 50
[]

[Outputs]
  exodus = true
[]
