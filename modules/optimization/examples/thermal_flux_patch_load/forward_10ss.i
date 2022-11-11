[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = halfSphere.e
  []
[]

[Variables]
  [temperature]
  []
[]

[AuxVariables]
  [saved_t]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [heat_conduction]
    type = ADHeatConduction
    variable = temperature
    save_in = saved_t
  []
[]

[BCs]
  [round]
    type = ConvectiveFluxFunction
    boundary = round
    variable = temperature
    coefficient = 0.05
    T_infinity = 100.0
  []
  [flat_0]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = 100
    postprocessor = p0
  []
  [flat_1]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = 101
    postprocessor = p1
  []
  [flat_2]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = 102
    postprocessor = p2
  []
  [flat_3]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = 103
    postprocessor = p3
  []
  [flat_4]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = 104
    postprocessor = p4
  []
  [flat_5]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = 105
    postprocessor = p5
  []
  [flat_6]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = 106
    postprocessor = p6
  []
  [flat_7]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = 107
    postprocessor = p7
  []
  [flat_8]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = 108
    postprocessor = p8
  []
  [flat_9]
    type = PostprocessorNeumannBC
    variable = temperature
    boundary = 109
    postprocessor = p9
  []
[]

[Postprocessors]
  [p0]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p1]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p2]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p3]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p4]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p5]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p6]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p7]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p8]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
  [p9]
    type = ConstantValuePostprocessor
    value = 0
    execute_on = LINEAR
  []
[]

[Materials]
  [steel]
    type = ADGenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 5
  []
[]

[Problem] #do we need this
  type = FEProblem
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-8
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
  nl_forced_its = 1
[]

[VectorPostprocessors]
  #-----every forward problem should have these two
  [data_pt]
    type = VppPointValueSampler
    variable = temperature
    reporter_name = measure_data
  []
[]

[Reporters]
  [measure_data]
    type = OptimizationData
  []
[]

[Controls]
  [parameterReceiver]
    type = ControlsReceiver
  []
[]

[Outputs]
  console = true #false
  exodus = true
  csv = true
  file_base = 'forward'
[]
