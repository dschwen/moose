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
  # This variable is conserverd when coarsening the mesh
  [./u]
    [./InitialCondition]
      type = FunctionIC
      function = 'x^2+y^2'
    [../]
  [../]
  # This variable is not conserverd when coarsening the mesh
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
    [./all]
      type = UniformMarker
      mark = COARSEN
    [../]
  [../]
  marker = all
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
