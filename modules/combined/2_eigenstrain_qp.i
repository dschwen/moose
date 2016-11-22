#
# Eigenstrain with neighbor relation of quasiperiodicity
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 50
  xmin = -0.5
  xmax = 0.5

  ymin = -0.5
  ymax = 0.5

[]

[UserObjects]
  [./quasiperiodicneighbors_x]
    type = QuasiPeriodicNeighbors
    execute_on = 'initial linear nonlinear timestep_begin'
    component = 0
  [../]
  [./quasiperiodicneighbors_y]
    type = QuasiPeriodicNeighbors
    execute_on = 'initial linear nonlinear timestep_begin'
    component = 1
  [../]
[]

[MeshModifiers]
  [./cnode]
    type = AddExtraNodeset
    coord = '0.0 0.0'
    new_boundary = 100
  [../]

  [./anode]
    type = AddExtraNodeset
    coord = '0.0 0.5'
    new_boundary = 101
  [../]


  [./cornernode_1]
    type = AddExtraNodeset
    coord = '-0.5 0.0'
    new_boundary = 102
  [../]
  [./cornernode_2]
    type = AddExtraNodeset
    coord = '0.0 -0.5'
    new_boundary = 103
  [../]
  [./cornernode_3]
    type = AddExtraNodeset
    coord = '0.5 0.0'
    new_boundary = 104
  [../]
  [./cornernode_4]
    type = AddExtraNodeset
    coord = '0.5 0.5'
    new_boundary = 105
  [../]

[]

[GlobalParams]
  penalty = 1.0 #10 with du/dt on seems pretty good, problems with solve after some time
  derivative_order = 2
  enable_jit = true
  D = 1.0
  D_neighbor = 1.0
  displacements = 'disp_x disp_y'
  use_displaced_mesh = false
[]

[Variables]
  # Solute concentration variable
  [./c]
    [./InitialCondition]
      type = RandomIC
      seed = 2
      min = 0.49
      max = 0.51
    [../]
    block = 0
  [../]
  [./w]
    block = 0
  [../]

  # Mesh displacement
  [./disp_x]
    block = 0
    #[./InitialCondition]
    #  type = RandomIC
    #  min = -1e-4
    #  max = 1e-4
    #[../]
  [../]
  [./disp_y]
    block = 0
    #[./InitialCondition]
    #  type = RandomIC
    #  min = -1e-4
    #  max = 1e-4
    #[../]
  [../]
[]

# AuxVars to compute the free energy density for outputting
[AuxVariables]
  [./local_energy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xx_elastic]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy_elastic]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xy_elastic]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xz_elastic]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_zz_elastic]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yz_elastic]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_xx_elastic]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_yy_elastic]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_xy_elastic]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_xz_elastic]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_zz_elastic]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./strain_yz_elastic]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./R_disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./R_disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxKernels]
  [./comp_R_disp_x]
    type = DebugResidualAux
    block = 0
    execute_on = 'timestep_end'
    variable = R_disp_x
    debug_variable = disp_x
  [../]
  [./comp_R_disp_y]
    type = DebugResidualAux
    block = 0
    execute_on = 'timestep_end'
    variable = R_disp_y
    debug_variable = disp_y
  [../]

  [./local_free_energy]
    type = TotalFreeEnergy
    block = 0
    execute_on = 'initial LINEAR'
    variable = local_energy
    interfacial_vars = 'c'
    kappa_names = 'kappa_c'
  [../]
  [./matl_e11]
    type = RankTwoAux
    rank_two_tensor = elastic_strain
    index_i = 0
    index_j = 0
    variable = strain_xx_elastic
    block = 0
    execute_on = 'timestep_end'
  [../]
  [./matl_e12]
    type = RankTwoAux
    rank_two_tensor = elastic_strain
    index_i = 0
    index_j = 1
    variable = strain_xy_elastic
    block = 0
    execute_on = 'timestep_end'
  [../]
  [./matl_e13]
    type = RankTwoAux
    rank_two_tensor = elastic_strain
    index_i = 0
    index_j = 2
    variable = strain_xz_elastic
    block = 0
    execute_on = 'timestep_end'
  [../]
  [./matl_e22]
    type = RankTwoAux
    rank_two_tensor = elastic_strain
    index_i = 1
    index_j = 1
    variable = strain_yy_elastic
    block = 0
    execute_on = 'timestep_end'
  [../]
  [./matl_e23]
    type = RankTwoAux
    rank_two_tensor = elastic_strain
    index_i = 1
    index_j = 2
    variable = strain_yz_elastic
    block = 0
    execute_on = 'timestep_end'
  [../]
  [./matl_e33]
    type = RankTwoAux
    rank_two_tensor = elastic_strain
    index_i = 2
    index_j = 2
    variable = strain_zz_elastic
    block = 0
    execute_on = 'timestep_end'
  [../]
[]

[Kernels]
  # Set up stress divergence kernels
  [./TensorMechanics]
  [../]

  # Cahn-Hilliard kernels
  [./c_dot]
    type = CoupledTimeDerivative
    variable = w
    v = c
  [../]
  #[./disp_x_dot]
  #  type = TimeDerivative
  #  variable = disp_x
  #[../]
  #[./disp_y_dot]
  #  type = TimeDerivative
  #  variable = disp_y
  #[../]
  [./c_res]
    type = SplitCHParsed
    variable = c
    f_name = F
    kappa_name = kappa_c
    w = w
  [../]
  [./w_res]
    type = SplitCHWRes
    variable = w
    mob_name = M
  [../]
[]

[Materials]
  # declare a few constants, such as mobilities (L,M) and interface gradient prefactors (kappa*)
  [./consts]
    type = GenericConstantMaterial
    block = '0'
    prop_names  = 'M   kappa_c'
    prop_values = '0.2 0.01   '
  [../]

  [./shear1]
    type = GenericConstantRankTwoTensor
    block = 0
    tensor_values = '0 0 0 0 0 0.5'
    tensor_name = shear1
  [../]
  [./shear2]
    type = GenericConstantRankTwoTensor
    block = 0
    tensor_values = '0 0 0 0 0 -0.5'
    tensor_name = shear2
  [../]
  [./expand3]
    type = GenericConstantRankTwoTensor
    block = 0
    tensor_values = '1 1 0 0 0 0'
    tensor_name = expand3
  [../]

  [./weight1]
    type = DerivativeParsedMaterial
    block = 0
    function = '0.3*c^2'
    f_name = weight1
    args = c
  [../]
  [./weight2]
    type = DerivativeParsedMaterial
    block = 0
    function = '0.3*(1-c)^2'
    f_name = weight2
    args = c
  [../]
  [./weight3]
    type = DerivativeParsedMaterial
    block = 0
    function = '4*(0.5-c)^2'
    f_name = weight3
    args = c
  [../]

  # matrix phase
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    C_ijkl = '1 1'
    fill_method = symmetric_isotropic
  [../]
  [./strain]
    type = ComputeSmallStrain
    block = 0
    displacements = 'disp_x disp_y'
    eigenstrain_names = eigenstrain
  [../]

  [./eigenstrain]
    type = CompositeEigenstrain
    block = 0
    tensors = 'shear1  shear2  expand3'
    weights = 'weight1 weight2 weight3'
    args = c
    eigenstrain_name = eigenstrain
  [../]
  # chemical free energies
  [./chemical_free_energy]
    type = DerivativeParsedMaterial
    block = 0
    f_name = Fc
    function = '4*c^2*(1-c)^2'
    args = 'c'
    outputs = exodus
    output_properties = Fc
  [../]

  [./stress]
    type = ComputeLinearElasticStress
    block = 0
  [../]

  # elastic free energies
  [./elastic_free_energy]
    type = ElasticEnergyMaterial
    f_name = Fe
    block = 0
    args = 'c'
    outputs = exodus
    output_properties = Fe
  [../]

  # free energy (chemical + elastic)
  [./free_energy]
    type = DerivativeSumMaterial
    block = 0
    f_name = F
    sum_materials = 'Fc Fe'
    args = 'c'
  [../]
[]

[InterfaceKernels]
  [./xx_int_left]
    type = QuasiPeriodicTestKernel
    variable = disp_x
    neighbor_var = disp_x
    component = 0
    boundary = 'left'
  [../]

  [./yx_int_left]
    type = QuasiPeriodicTestKernel
    variable = disp_y
    neighbor_var = disp_y
    component = 0
    boundary = 'left'
  [../]

  ## -----top-----
  [./xy_int_top]
    type = QuasiPeriodicTestKernel
    variable = disp_x
    neighbor_var = disp_x
    component = 1
    boundary = 'top'
  [../]

  [./yy_int_top]
    type = QuasiPeriodicTestKernel
    variable = disp_y
    neighbor_var = disp_y
    component = 1
    boundary = 'top'
  [../]

[]

[BCs]
  [./Periodic]
    [./up_down]
      primary = top
      secondary = bottom
      translation = '0 -1.0 0'
      variable = 'c w'
    [../]
    [./left_right]
      primary = left
      secondary = right
      translation = '1.0 0 0'
      variable = 'c w'
    [../]
  [../]

  # fix center point location
  [./centerfix_x]
    type = PresetBC
    boundary = 100
    variable = disp_x
    value = 0.0
  [../]
  [./centerfix_y]
    type = PresetBC
    boundary = 100
    variable = disp_y
    value = 0.0
  [../]


  ## fix side point x coordinate to inhibit rotation
  [./angularfix_1]
    type = PresetBC
    boundary = 101
    variable = disp_x
    value = 0.0
  [../]
[]

# We monitor the total free energy and the total solute concentration (should be constant)
[Postprocessors]
  [./total_free_energy]
    type = ElementIntegralVariablePostprocessor
    block = 0
    execute_on = 'initial TIMESTEP_END'
    variable = local_energy
  [../]
  [./total_solute]
    type = ElementIntegralVariablePostprocessor
    block = 0
    execute_on = 'initial TIMESTEP_END'
    variable = c
  [../]
  [./c_min]
    type = ElementExtremeValue
    block = 0
    execute_on = 'initial TIMESTEP_END'
    value_type = min
    variable = c
  [../]
  [./c_max]
    type = ElementExtremeValue
    block = 0
    execute_on = 'initial TIMESTEP_END'
    value_type = max
    variable = c
  [../]
[]


[Preconditioning]
  [./SMP]
    type = SMP
    full = true
    # mortar currently does not support MPI parallelization
    petsc_options_iname = '-pc_type'
    #does the null space mess this problem up??
    petsc_options_value = ' lu'
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2 #does not seem to matter if bdf2 or implicit-euler
  solve_type = 'PJFNK'

  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10

  start_time = 0.0
  num_steps = 500


  #nl_max_its = 15
  #end_time = 0.05

  [./TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 0.01
  [../]
  #dt = 0.001
  dtmax = 0.01
[]

[Outputs]
  execute_on = 'timestep_end'
  print_linear_residuals = true
  exodus = true
  file_base = out_test_strain
  [./table]
    type = CSV
    delimiter = ' '
  [../]
[]
