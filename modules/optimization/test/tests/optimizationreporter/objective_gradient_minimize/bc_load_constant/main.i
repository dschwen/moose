[StochasticTools]
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 20
  xmax = 1
  ymax = 2
  bias_x = 1.0
  bias_y = 1.0
[]

[OptimizationReporter]
  type = ObjectiveGradientMinimize
  parameter_names = 'bc_left bc_right'
  num_values = '1 1'
  measurement_points = '0.2 0.2 0
            0.8 0.6 0
            0.2 1.4 0
            0.8 1.8 0'
  measurement_values = '199 214 154 129'
[]

[Executioner]
  type = Optimize
  tao_solver = taolmvm
  petsc_options_iname = '-tao_gatol -tao_max_it' #-tao_ls_type'
  petsc_options_value = '1e-1 50' #unit'
  verbose = true
[]

[MultiApps]
  [forward]
    type = OptimizeFullSolveMultiApp
    input_files = forward.i
    execute_on = "FORWARD"
    clone_parent_mesh = true
  []
  [adjoint]
    type = OptimizeFullSolveMultiApp
    input_files = adjoint.i
    execute_on = "ADJOINT"
    clone_parent_mesh = true
  []
[]

[Transfers]
  #these are usually the same for all input files.
  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'data_pt/temperature data_pt/temperature'
    to_reporters = 'OptimizationReporter/simulation_values receiver/measured'
  []
  [toAdjoint]
    type = MultiAppReporterTransfer
    to_multi_app = adjoint
    from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord OptimizationReporter/misfit_values'
    to_reporters = 'misfit/measurement_xcoord misfit/measurement_ycoord misfit/measurement_zcoord misfit/misfit_values'
  []
  [toForward_measument]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord'
    to_reporters = 'measure_data/measurement_xcoord measure_data/measurement_ycoord measure_data/measurement_zcoord'
  []
  #these are different,
  # - to forward depends on teh parameter being changed
  # - from adjoint depends on the gradient being computed from the adjoint
  #NOTE:  the adjoint variable we are transferring is actually the gradient

  [toForward]
    type = OptimizationParameterTransfer
    to_multi_app = forward
    value_names = 'bc_left bc_right'
    parameters = 'BCs/left/value BCs/right/value'
    to_control = parameterReceiver
  []
  [fromAdjoint]
    type = MultiAppReporterTransfer
    from_multi_app = adjoint
    from_reporters = 'adjoint_pt/adjoint_pt'
    to_reporters = 'OptimizationReporter/adjoint'
  []
[]

[Reporters]
  [receiver]
    type = ConstantReporter
    real_vector_names = measured
    real_vector_values = '0'
  []
  [optInfo]
    type = OptimizationInfo
    items = 'gnorm function_value'
  []
[]

[Outputs]
  csv = true
[]
