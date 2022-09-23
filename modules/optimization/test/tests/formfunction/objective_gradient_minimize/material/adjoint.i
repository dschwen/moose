[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmax = 2
  ymax = 2
[]

[Variables]
  [temperature]
    order = FIRST
    family = LAGRANGE
  []
[]
[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = temperature
  []
[]

[DiracKernels]
  [pt]
    type = VectorPostprocessorPointSource
    variable = temperature
    vector_postprocessor = point_source
    x_coord_name = x
    y_coord_name = y
    z_coord_name = z
    value_name = value
  []
[]

[AuxVariables]
  [temperature_forward]
    order = FIRST
    family = LAGRANGE
  []
[]

[BCs]
  [left]
    type = NeumannBC
    variable = temperature
    boundary = left
    value = 0
  []
  [right]
    type = NeumannBC
    variable = temperature
    boundary = right
    value = 0
  []
  [bottom]
    type = DirichletBC
    variable = temperature
    boundary = bottom
    value = 0
  []
  [top]
    type = DirichletBC
    variable = temperature
    boundary = top
    value = 0
  []
[]

[Functions]
  [thermo_conduct]
    type = ParsedFunction
    value = alpha
    vars = 'alpha'
    vals = 'p1'
  []
[]

[Materials]
  [steel]
    type = GenericFunctionMaterial
    prop_names = 'thermal_conductivity'
    prop_values = 'thermo_conduct'
  []
  [volumetric_heat]
    type = ADGenericFunctionMaterial
    prop_names = 'volumetric_heat'
    prop_values = '1000'
  []
[]

[Problem]
  type = FEProblem
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  petsc_options_iname = '-ksp_type -pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'preonly lu       superlu_dist'
[]

[Postprocessors]
  [pp_adjoint_grad]
    # integral of load function gradient w.r.t parameter
    type = DiffusionVariableIntegral
    variable = temperature
    variable_forward = temperature_forward
  []
  [p1]
    type = ConstantValuePostprocessor
    value = 10
    execute_on = 'initial linear'
  []
[]

[VectorPostprocessors]
  [point_source]
    type = ConstantVectorPostprocessor
    vector_names = 'x y z value'
    value = '0.2 0.8 0.2 0.8;
             0.2 0.6 1.4 1.8;
             0   0   0   0;
             10  10  10  10'
  []
  [adjoint_grad]
    type = VectorOfPostprocessors
    postprocessors = 'pp_adjoint_grad'
  []
[]

[Controls]
  [parameterReceiver]
    type = ControlsReceiver
  []
[]

[Outputs]
  console = false
  exodus = false
  file_base = 'adjoint'
[]
