# Coupled thermomechanical two-block model from moosify_nnl.pdf.
# The placeholder UMAT is incremental elasticity only. Replace it with the specified
# viscoplastic UMAT when its Z evolution law and temperature contract are available.
# The placeholder has no STATEV entries; expose the final UMAT's Z through its state_var
# material property after setting its required num_state_vars value.

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  temperature = temperature
[]

[Mesh]
  [generated]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 9
    ny = 6
    nz = 4
    xmin = -0.09
    xmax = 0.09
    ymin = -0.06
    ymax = 0.06
    zmin = -0.04
    zmax = 0.04
  []
  [block_a]
    type = SubdomainBoundingBoxGenerator
    input = generated
    block_id = 1
    block_name = block_a
    bottom_left = '-0.09 -0.06 -0.04'
    top_right = '0.09 0.06 0'
  []
  [block_b]
    type = RenameBlockGenerator
    input = block_a
    old_block = 0
    new_block = block_b
  []
[]

[Variables]
  [temperature]
    initial_condition = 20
  []
  [global_strain]
    family = SCALAR
    order = SIXTH
  []
[]

[AuxVariables]
  [phi]
    initial_condition = 0
  []
[]

[AuxKernels]
  [phi]
    type = FunctionAux
    variable = phi
    function = phi_function
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Functions]
  [phi_function]
    type = ParsedFunction
    # x is in meters. 0.0081 m2 is 9 cm squared, the PDF normalization in SI units.
    expression = 'if(t <= 1000, 3000 - 2.9 * t - x^2 / 0.0081 + 1, if(t < 100000, 100 - x^2 / 0.0081 + 1, 0))'
  []
  [pressure_function]
    type = ParsedFunction
    expression = 'if(t < 100000, -1e5, 0)'
  []
  [ambient_temperature]
    type = ParsedFunction
    expression = 'if(t < 100000, 500, 20)'
  []
  [heat_transfer_coefficient]
    type = ParsedFunction
    expression = '200 * sin(pi * t / 1e6)'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = true
    strain = FINITE
    incremental = true
    global_strain = global_strain
    eigenstrain_names = thermal_expansion
    generate_output = 'stress_xx stress_yy stress_zz strain_xx strain_yy strain_zz'
  []
[]

[Physics/SolidMechanics/GlobalStrain]
  [prp]
    scalar_global_strain = global_strain
    displacements = 'disp_x disp_y disp_z'
    auxiliary_displacements = 'total_disp_x total_disp_y total_disp_z'
    global_displacements = 'global_disp_x global_disp_y global_disp_z'
    active_directions = 'x y'
  []
[]

[Kernels]
  [heat_time]
    type = ADHeatConductionTimeDerivative
    variable = temperature
    density_name = density
    specific_heat = specific_heat
  []
  [heat_conduction]
    type = ADHeatConduction
    variable = temperature
    thermal_conductivity = thermal_conductivity
  []
  [heat_source]
    type = ADMatHeatSource
    variable = temperature
    material_property = volumetric_heat
    block = block_a
  []
[]

[BCs]
  [bottom_z]
    type = DirichletBC
    variable = disp_z
    boundary = bottom
    value = 0
  []
  [left_x]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [back_y]
    type = DirichletBC
    variable = disp_y
    boundary = back
    value = 0
  []
  # The local normal displacements vanish on these PRP faces; their homogeneous
  # normal motion is supplied by the x/y Global Strain components.
  [right_x]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 0
  []
  [front_y]
    type = DirichletBC
    variable = disp_y
    boundary = front
    value = 0
  []
  [top_pressure]
    type = FunctionNeumannBC
    variable = disp_z
    boundary = top
    function = pressure_function
  []
  [top_convection]
    type = ADConvectiveHeatFluxBC
    variable = temperature
    boundary = top
    T_infinity = ambient_temperature
    heat_transfer_coefficient = heat_transfer_coefficient
  []
[]

[Materials]
  [block_a_thermal]
    type = ADGenericConstantMaterial
    block = block_a
    prop_names = 'density specific_heat thermal_conductivity'
    prop_values = '8000 500 80'
  []
  [block_b_thermal]
    type = ADGenericConstantMaterial
    block = block_b
    prop_names = 'density specific_heat thermal_conductivity volumetric_heat'
    prop_values = '3000 800 200 0'
  []
  [block_a_heat_source]
    type = ADParsedMaterial
    block = block_a
    property_name = volumetric_heat
    coupled_variables = phi
    expression = 'phi^2'
  []
  [top_convection_properties]
    type = ADGenericFunctionMaterial
    boundary = top
    prop_names = 'ambient_temperature heat_transfer_coefficient'
    prop_values = 'ambient_temperature heat_transfer_coefficient'
  []
  [block_a_thermal_expansion]
    type = ComputeThermalExpansionEigenstrain
    block = block_a
    temperature = temperature
    thermal_expansion_coeff = 1e-6
    stress_free_temperature = 20
    eigenstrain_name = thermal_expansion
  []
  [umat]
    type = AbaqusUMATStress
    block = block_a
    constant_properties = '2e11 0.3'
    plugin = '../../../solid_mechanics/test/plugins/elastic_incremental'
    num_state_vars = 0
    temperature = temperature
    external_fields = phi
    use_one_based_indexing = true
  []
  [block_b_thermal_expansion]
    type = ComputeThermalExpansionEigenstrain
    block = block_b
    temperature = temperature
    thermal_expansion_coeff = 5e-6
    stress_free_temperature = 20
    eigenstrain_name = thermal_expansion
  []
  [block_b_elasticity]
    type = ComputeIsotropicElasticityTensor
    block = block_b
    youngs_modulus = 4e11
    poissons_ratio = 0.15
  []
  [block_b_stress]
    type = ComputeFiniteStrainElasticStress
    block = block_b
  []
[]

[UserObjects]
  [analysis_steps]
    type = AnalysisStepUserObject
    step_start_times = '0 500 100000'
  []
[]

[Postprocessors]
  [top_reaction]
    type = SidesetReaction
    direction = '0 0 1'
    stress_tensor = stress
    boundary = top
  []
  [top_heat_loss]
    type = ADConvectiveHeatTransferSideIntegral
    T_fluid = ambient_temperature
    htc = heat_transfer_coefficient
    T_solid = temperature
    boundary = top
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  automatic_scaling = true
  petsc_options_iname = '-pc_type'
  petsc_options_value = lu
  l_max_its = 30
  nl_max_its = 20
  start_time = 0
  end_time = 105000
  dt = 100
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-5
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
