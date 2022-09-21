[Mesh]
[]

[Variables]
  [temperature]
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = temperature
  []
[]

[BCs]
  [left_constant]
    type = FunctionNeumannBC
    variable = temperature
    boundary = left
    function = left_constant_function
  []
  [left_linear]
    type = FunctionNeumannBC
    variable = temperature
    boundary = left
    function = left_linear_function
  []
  [right]
    type = NeumannBC
    variable = temperature
    boundary = right
    value = 100
  []
  [bottom]
    type = DirichletBC
    variable = temperature
    boundary = bottom
    value = 200
  []
  [top]
    type = DirichletBC
    variable = temperature
    boundary = top
    value = 100
  []
[]

[Functions]
  [left_constant_function]
    type = ParsedFunction
    value = a*1.0
    vars = 'a'
    vals = 'p1'
  []
  [left_linear_function]
    type = ParsedFunction
    value = b*y
    vars = 'b'
    vals = 'p2'
  []
[]

[Materials]
  [steel]
    type = GenericConstantMaterial
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
  [data_pt]
    type = MeasuredDataPointSampler
    variable = temperature
    points = '0.2 0.2 0
              0.8 0.6 0
              0.2 1.4 0
              0.8 1.8 0'
    measured_values = '207 204 185 125'
  []
[]


[Postprocessors]
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
[]

[Controls]
  [parameterReceiver]
    type = ControlsReceiver
  []
[]

[Outputs]
  console = true
  exodus = true
  file_base = 'forward'
[]
