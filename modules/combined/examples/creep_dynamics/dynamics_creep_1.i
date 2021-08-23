[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  ymax = 3
  ny = 3
  dim = 3
[]

[Variables]
  [T]
  []
[]

[ICs]
  [T]
    type = FunctionIC
    variable = T
    function = '1000-y/3*100'
  []
[]

[Modules/TensorMechanics/Master]
  [all]
    strain = FINITE
    incremental = true
    add_variables = true
    generate_output = 'stress_yy creep_strain_xx creep_strain_yy creep_strain_zz elastic_strain_yy'
    use_automatic_differentiation = true
  []
[]

[Functions]
  [intensity]
    type = PiecewiseLinear
    xy_data = '0 0 1 1 2 1 3 0'
  []
  [T_top]
    type = ParsedFunction
    value = '900 + 200*sin(2 * t * pi)'
  []
[]

[Kernels]
  [heat]
    type = ADHeatConduction
    variable = T
  []
  [heat_ie]
    type = ADHeatConductionTimeDerivative
    variable = T
  []
[]

[BCs]
  [u_top_pull]
    type = ADPressure
    variable = disp_y
    component = 1
    boundary = top
    function = intensity
    constant = -10.0e6
  []
  [u_bottom_fix]
    type = ADDirichletBC
    variable = disp_y
    boundary = bottom
    value = 0.0
  []
  [u_yz_fix]
    type = ADDirichletBC
    variable = disp_x
    boundary = left
    value = 0.0
  []
  [u_xy_fix]
    type = ADDirichletBC
    variable = disp_z
    boundary = back
    value = 0.0
  []
  [T_fix]
    type = DirichletBC
    variable = T
    boundary = 'bottom'
    value = 1000.0
  []
  [T_fix_top]
    type = FunctionDirichletBC
    variable = T
    boundary = 'top'
    function = T_top
  []
[]

[Materials]
  [elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 2e11
    poissons_ratio = 0.3
    constant_on = SUBDOMAIN
  []
  [radial_return_stress]
    type = ADComputeMultipleInelasticStress
    inelastic_models = 'power_law_creep'
  []
  [power_law_creep]
    type = ADPowerLawCreepStressUpdate
    coefficient = 1.0e-15
    n_exponent = 4
    activation_energy = 2.8e5
    temperature = T
  []

  [thermal]
    type = ADHeatConductionMaterial
    specific_heat = 1.0
    thermal_conductivity = 1.
  []
  [density]
    type = ADDensity
    density = 1.0
  []
[]

[Postprocessors]
  [intensity]
    type = FunctionValuePostprocessor
    function = intensity
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T_top]
    type = FunctionValuePostprocessor
    function = T_top
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [ycreep]
    type = ElementAverageValue
    variable = creep_strain_yy
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [delta_l]
    type = SideAverageValue
    variable = disp_y
    boundary = top
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T_center]
    type = PointValue
    variable = T
    point = '0.5 1.5 0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp'
  petsc_options_iname = '-ksp_gmres_restart'
  petsc_options_value = '101'

  line_search = 'none'

  l_max_its = 20
  nl_max_its = 20
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-6
  l_tol = 1e-5

  dt = 0.05
  start_time = 0.0
  end_time = 3.0
[]

[Outputs]
  print_linear_residuals = false
  checkpoint = true
  exodus = true
  csv = true
[]
