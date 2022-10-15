[StochasticTools]
[]

[OptimizationReporter]
  type = OptimizationReporter
  parameter_names = 'parameter_results'
  num_values = '8'

  adjoint_data_name = adjoint
[]

[Executioner]
  type = Optimize
  # tao_solver = taonm
  # petsc_options_iname='-tao_gatol'
  # petsc_options_value='1e-2'
  tao_solver = taolmvm #TAOOWLQN #TAOBMRM #taolmvm #taobncg
  petsc_options_iname = '-tao_gatol'# -tao_cg_delta_max'
  petsc_options_value = '1e-2'
  # tao_solver = taontr
  # petsc_options_iname='-tao_fd_hessian -tao_fd_delta -tao_ntr_min_radius -tao_ntr_max_radius -tao_ntr_init_type -tao_gatol'
  # petsc_options_value='true 0.000001 0 1e16 constant 1e-2'
  verbose = true
[]

[MultiApps]
  [forward]
    type = OptimizeFullSolveMultiApp
    input_files = forward.i
    execute_on = "FORWARD"
  []
  [adjoint]
    type = OptimizeFullSolveMultiApp
    input_files = adjoint.i
    execute_on = "ADJOINT"
  []
[]

[Transfers]
  [toforward]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/parameter_results'
    to_reporters = 'point_source/value'
  []
  [fromforward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'dr/temperature_difference dr/temperature'
    to_reporters = 'OptimizationReporter/misfit measured/values'
  []

  [toadjoint]
    type = MultiAppReporterTransfer
    to_multi_app = adjoint
    from_reporters = 'OptimizationReporter/misfit'
    to_reporters = 'point_source/value'
  []
  [fromadjoint]
    type = MultiAppReporterTransfer
    from_multi_app = adjoint
    from_reporters = 'ar/temperature'
    to_reporters = 'OptimizationReporter/adjoint'
  []
[]

[Reporters]
  [measured]
    type = ConstantReporter
    real_vector_names = values
    real_vector_values = '0'
  []
[]

[Outputs]
  csv=true
[]
