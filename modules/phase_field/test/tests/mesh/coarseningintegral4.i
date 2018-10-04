[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
  xmin = -1
  ymin = -1
  uniform_refine = 6
[]

[AuxVariables]
  [./coarsen]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./u_comp]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./coarsen]
    type = FunctionAux
    variable = coarsen
    function = 's:=int(t*1e4)%2;if((s=0&x>=0)|(s!=0&x<0),1,0)'
  [../]
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
    type = MatDiffusion
    variable = u
    D_name = 0
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
    type = MatDiffusion
    variable = v
    D_name = 0
  [../]
[]

[Adaptivity]
  [./Markers]
    [./steps]
      # with the AuxKernel above this alternates between coarsening and refinement
      type = ValueThresholdMarker
      variable = coarsen
      refine = 2
      coarsen = 0.5
    [../]
  [../]
  marker = steps
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
  [./U0]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = initial
  [../]
  [./V0]
    type = ElementIntegralVariablePostprocessor
    variable = v
    execute_on = initial
  [../]
  [./dU]
    type = DifferencePostprocessor
    value1 = U
    value2 = U0
    execute_on = 'initial timestep_end'
  [../]
  [./dV]
    type = DifferencePostprocessor
    value1 = V
    value2 = V0
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
  hide = 'U V U0 V0'
[]
