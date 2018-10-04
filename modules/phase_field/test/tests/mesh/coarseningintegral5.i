[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 8
  ny = 8
  xmin = 0.0
  xmax = 50
  ymin = 0.0
  ymax = 50
  elem_type = QUAD4
  uniform_refine = 3
[]

[AuxVariables]
  [./ccomp]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./proc]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./ccomp]
    type = CoarseningIntegralAux
    execute_on = timestep_begin
    variable = ccomp
    tracker = comp
  [../]
  [./proc]
    type = ProcessorIDAux
    variable = proc
  [../]
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = CrossIC
      x1 = 0.0
      x2 = 30.0
      y1 = 0.0
      y2 = 30.0
    [../]
  [../]
  [./w]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    coupled_groups = 'c,w'
  [../]
[]

[Kernels]
  [./cres]
    type = SplitCHMath
    variable = c
    kappa_name = kappa_c
    w = w
  [../]
  [./wres]
    type = SplitCHWRes
    variable = w
    mob_name = M
  [../]
  [./time]
    type = CoupledTimeDerivative
    variable = w
    v = c
  [../]
  [./src_c]
    type = CoarseningIntegralCompensation
    variable = w
    tracker = comp
  [../]
[]

[BCs]
  [./Periodic]
    [./auto]
      auto_direction = 'x y'
      variable = 'c w'
    [../]
  [../]
[]

[Materials]
  [./constant]
    type = GenericConstantMaterial
    prop_names  = 'M kappa_c'
    prop_values = '1.0 2.0'
    block = 0
  [../]
[]

[Adaptivity]
  [./Indicators]
    [./gradc]
      type = GradientJumpIndicator
      variable = c
    [../]
  [../]
  [./Markers]
    [./mark]
      type = ErrorFractionMarker
      indicator = gradc
      refine = 0.8
      coarsen = 0.2
    [../]
  [../]
  marker = mark
  initial_steps = 3
  max_h_level = 5
[]

[Postprocessors]
  [./C]
    type = ElementIntegralVariablePostprocessor
    variable = c
    execute_on = 'initial timestep_end'
  [../]
  [./C0]
    type = ElementIntegralVariablePostprocessor
    variable = c
    execute_on = initial
  [../]
  [./dC]
    type = DifferencePostprocessor
    value1 = C
    value2 = C0
    execute_on = 'initial timestep_end'
  [../]
[]

[UserObjects]
  [./comp]
    type = CoarseningIntegralTracker
    v = c
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'BDF2'
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  l_max_its = 30
  l_tol = 1.0e-3

  nl_max_its = 50
  nl_rel_tol = 1.0e-10

  dt = 1.0
  num_steps = 40
[]

[Outputs]
  exodus = true
  csv = true
  hide = 'C C0'
[]
