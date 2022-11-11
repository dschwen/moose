[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 20
  xmax = 1
  ymax = 2
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [heat_conduction]
    type = ADHeatConduction
    variable = temperature
  []
[]

[DiracKernels]
  [pt]
    type = ReporterPointSource
    variable = temperature
    x_coord_name = 'point_source/x'
    y_coord_name = 'point_source/y'
    z_coord_name = 'point_source/z'
    value_name = 'point_source/value'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = temperature
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
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

[Materials]
  [steel]
    type = ADGenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 5
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[VectorPostprocessors]
  [point_source]
    type = ConstantVectorPostprocessor
    vector_names = 'x y z value'
    value = '0.2 0.2 0.8;
             0.2 0.8 0.2;
             0 0 0;
             -2458 7257 26335'
    execute_on = LINEAR
  []
  #-----every forward problem should have these two
  [data_pt]
    type = VppPointValueSampler
    variable = temperature
    reporter_name = measure_data
    # x_coord_name = measure_data/measurement_xcoord
    # y_coord_name = measure_data/measurement_ycoord
    # z_coord_name = measure_data/measurement_zcoord
  []
  [vertical]
    type = LineValueSampler
    variable = 'temperature'
    start_point = '0.5 0 0'
    end_point = '0.5 1.4 0'
    num_points = 21
    sort_by = y
  []
[]

[Reporters]
  [measure_data]
    type = OptimizationData
  []
[]
#---------------------------------------------------

[Outputs]
  csv = true
  json = false
  console = true
  exodus = false
  file_base = 'forward'
[]
