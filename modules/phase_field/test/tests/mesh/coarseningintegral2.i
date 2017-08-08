[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmin = -1
  ymin = -1
  uniform_refine = 3
[]

[AuxVariables]
  [./u_comp]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./u_comp]
    type = CoarseningIntegralAux
    execute_on = timestep_begin
    variable = u_comp
    tracker = comp
  [../]
[]

[Variables]
  [./u]
    [./InitialCondition]
      type = FunctionIC
      function = 'x^2+y^2'
    [../]
  [../]
  [./v]
    [./InitialCondition]
      type = FunctionIC
      function = 'x^2+y^2'
    [../]
  [../]
[]

[Kernels]
  [./dt_u]
    type = TimeDerivative
    variable = u
  [../]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [./src_u]
    type = CoarseningIntegralCompensation
    variable = u
    tracker = comp
  [../]
  [./dt_v]
    type = TimeDerivative
    variable = v
  [../]
  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[AuxVariables]
  [./flip]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./flip]
    type = FunctionAux
    variable = flip
    function = 'int(t*1e4)%4/3.0'
  [../]
[]

[Adaptivity]
  [./Markers]
    [./alternate]
      # with the AuxKernel above this alternates between coarsening and refinement
      type = ValueThresholdMarker
      variable = flip
      refine = 0.55
      coarsen = 0.45
    [../]
  [../]
  marker = alternate
[]

[Postprocessors]
  [./U]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = 'initial timestep_end'
  [../]
  [./V]
    type = ElementIntegralVariablePostprocessor
    variable = v
    execute_on = 'initial timestep_end'
  [../]
[]

[UserObjects]
  [./comp]
    type = CoarseningIntegralTracker
    v = u
  [../]
[]

[Executioner]
  type = Transient
  dt = 1e-4
  num_steps = 6
  nl_abs_tol = 1e-9
[]

[Outputs]
  csv = true
  exodus = true
[]
