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

  uniform_refine = 2
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
    props = 'Fa Fb Fgap'
  [../]
[]

[Variables]
  # terminal concentrations for the miscibility gap
  [./xa]
    initial_condition = 0
  [../]
  [./xb]
    initial_condition = 1
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

  [./flat_a]
    type = StabilizeGradient
    variable = xa
    direction = '1 0 0'
  [../]
  [./flat_b]
    type = StabilizeGradient
    variable = xb
    direction = '1 0 0'
  [../]
[]

[Materials]
  [./Fa]
    type = DerivativeParsedMaterial
    f_name = Fa
    args = 'c T'
    function = '2*(c-0.2)^2 - T/4'
    outputs = exodus
    output_properties = Fa
  [../]
  [./Fb]
    type = DerivativeParsedMaterial
    f_name = Fb
    args = 'c T'
    function = '0.3 + 5*(0.9-c)^2 - T'
    outputs = exodus
    output_properties = Fb
  [../]

  [./Fxa]
    type = DerivativeParsedMaterial
    f_name = Fxa
    args = 'xa T'
    function = '2*(xa-0.2)^2 - T/4'
    outputs = exodus
    output_properties = Fxa
  [../]
  [./Fxb]
    type = DerivativeParsedMaterial
    f_name = Fxb
    args = 'xb T'
    function = '0.3 + 5*(0.9-xb)^2 - T'
    outputs = exodus
    output_properties = Fxb
  [../]

  [./Fgap]
    type = DerivativeParsedMaterial
    f_name = Fgap
    material_property_names = 'Fxa(xa) Fxb(xb)'
    function = '1/(xb-xa) * ((xb-c)*Fxa + (c-xa)*Fxb)
    '
    # +
      #if(c<xa,100*(xa-c),if(c>xb,100*(c-xb),0)) +
      #if(xa>xb,(xa-xb)*100,0)
    args = 'xa xb c'
    outputs = exodus
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

#[Preconditioning]
#  [./smp]
#    type = FDP
#    full = true
#  [../]
#[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  #line_search = none

  #petsc_options_iname = '-pc_type'
  #petsc_options_value = 'lu'

  nl_max_its = 2000
  nl_abs_tol = 1e-8
  l_max_its = 50
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
