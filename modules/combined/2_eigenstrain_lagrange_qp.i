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
[]

[GlobalParams]
  #penalty = 1.0 #10 with du/dt on seems pretty good, problems with solve after some time
  #D = 1.0
  #D_neighbor = 1.0
  derivative_order = 2
  enable_jit = true
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
  [../]
  [./w]
  [../]

  # Mesh displacement
  [./disp_x]
  [../]
  [./disp_y]
  [../]

  # Lagrange multipliers
  [./lagrange_e_xx_lr]
  [../]
  [./lagrange_e_xy_lr]
  [../]
  [./lagrange_e_yy_lr]
  [../]

  [./lagrange_e_xx_tb]
  [../]
  [./lagrange_e_xy_tb]
  [../]
  [./lagrange_e_yy_tb]
  [../]
[]

# AuxVars to compute the free energy density for outputting
[AuxVariables]
  [./local_energy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  #[./stress_xx_elastic]
  #  order = CONSTANT
  #  family = MONOMIAL
  #[../]
  #[./stress_yy_elastic]
  #  order = CONSTANT
  #  family = MONOMIAL
  #[../]
  #[./stress_xy_elastic]
  #  order = CONSTANT
  #  family = MONOMIAL
  #[../]
  #[./stress_xz_elastic]
  #  order = CONSTANT
  #  family = MONOMIAL
  #[../]
  #[./stress_zz_elastic]
  #  order = CONSTANT
  #  family = MONOMIAL
  #[../]
  #[./stress_yz_elastic]
  #  order = CONSTANT
  #  family = MONOMIAL
  #[../]
  #[./strain_xx_elastic]
  #  order = CONSTANT
  #  family = MONOMIAL
  #[../]
  #[./strain_yy_elastic]
  #  order = CONSTANT
  #  family = MONOMIAL
  #[../]
  #[./strain_xy_elastic]
  #  order = CONSTANT
  #  family = MONOMIAL
  #[../]
  #[./strain_xz_elastic]
  #  order = CONSTANT
  #  family = MONOMIAL
  #[../]
  #[./strain_zz_elastic]
  #  order = CONSTANT
  #  family = MONOMIAL
  #[../]
  #[./strain_yz_elastic]
  #  order = CONSTANT
  #  family = MONOMIAL
  #[../]
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
  #[./matl_e11]
  #  type = RankTwoAux
  #  rank_two_tensor = elastic_strain
  #  index_i = 0
  #  index_j = 0
  #  variable = strain_xx_elastic
  #  block = 0
  #  execute_on = 'timestep_end'
  #[../]
  #[./matl_e12]
  #  type = RankTwoAux
  #  rank_two_tensor = elastic_strain
  #  index_i = 0
  #  index_j = 1
  #  variable = strain_xy_elastic
  #  block = 0
  #  execute_on = 'timestep_end'
  #[../]
  #[./matl_e13]
  #  type = RankTwoAux
  #  rank_two_tensor = elastic_strain
  #  index_i = 0
  #  index_j = 2
  #  variable = strain_xz_elastic
  #  block = 0
  #  execute_on = 'timestep_end'
  #[../]
  #[./matl_e22]
  #  type = RankTwoAux
  #  rank_two_tensor = elastic_strain
  #  index_i = 1
  #  index_j = 1
  #  variable = strain_yy_elastic
  #  block = 0
  #  execute_on = 'timestep_end'
  #[../]
  #[./matl_e23]
  #  type = RankTwoAux
  #  rank_two_tensor = elastic_strain
  #  index_i = 1
  #  index_j = 2
  #  variable = strain_yz_elastic
  #  block = 0
  #  execute_on = 'timestep_end'
  #[../]
  #[./matl_e33]
  #  type = RankTwoAux
  #  rank_two_tensor = elastic_strain
  #  index_i = 2
  #  index_j = 2
  #  variable = strain_zz_elastic
  #  block = 0
  #  execute_on = 'timestep_end'
  #[../]
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
  [./null_l_xx_lr]
    type = NullKernel
    variable = lagrange_e_xx_lr
  [../]
  [./null_l_xy_lr]
    type = NullKernel
    variable = lagrange_e_xy_lr
  [../]
  [./null_l_yy_lr]
    type = NullKernel
    variable = lagrange_e_yy_lr
  [../]

  [./null_l_xx_tb]
    type = NullKernel
    variable = lagrange_e_xx_tb
  [../]
  [./null_l_xy_tb]
    type = NullKernel
    variable = lagrange_e_xy_tb
  [../]
  [./null_l_yy_tb]
    type = NullKernel
    variable = lagrange_e_yy_tb
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
  #[./xx_int_left]
  #  type = QuasiPeriodicTestKernel
  #  variable = disp_x
  #  neighbor_var = disp_x
  #  component = 0
  #  boundary = 'left'
  #[../]
  #
  #[./yx_int_left]
  #  type = QuasiPeriodicTestKernel
  #  variable = disp_y
  #  neighbor_var = disp_y
  #  component = 0
  #  boundary = 'left'
  #[../]
  #
  ### -----top-----
  #[./xy_int_top]
  #  type = QuasiPeriodicTestKernel
  #  variable = disp_x
  #  neighbor_var = disp_x
  #  component = 1
  #  boundary = 'top'
  #[../]
  #
  #[./yy_int_top]
  #  type = QuasiPeriodicTestKernel
  #  variable = disp_y
  #  neighbor_var = disp_y
  #  component = 1
  #  boundary = 'top'
  #[../]


#----------lagrange multiplier interface kernels

  [./iface_x_lr]
    type = InterfaceDiffusionBoundaryTerm
    boundary = 'left'
    variable = disp_x
    neighbor_var = disp_x
  [../]
  [./lambda_xx_lr]
    type = EqualGradientLagrangeMultiplier
    variable = lagrange_e_xx_lr
    boundary = 'left'
    element_var = disp_x
    neighbor_var = disp_x
    component = 0
  [../]
  [./constraint_xx_lr]
    type = EqualGradientLagrangeInterface
    boundary = 'left'
    lambda = lagrange_e_xx_lr
    variable = disp_x
    neighbor_var = disp_x
    component = 0
  [../]

  #[./iface_x_lr]
  #  type = InterfaceDiffusionBoundaryTerm
  #  boundary = 'left'
  #  variable = disp_x
  #  neighbor_var = disp_x
  #[../]
  [./lambda_xy_lr]
    type = EqualGradientLagrangeMultiplier
    variable = lagrange_e_xy_lr
    boundary = 'left'
    element_var = disp_x
    neighbor_var = disp_x
    component = 1
  [../]
  [./constraint_xy_lr]
    type = EqualGradientLagrangeInterface
    boundary = 'left'
    lambda = lagrange_e_xy_lr
    variable = disp_x
    neighbor_var = disp_x
    component = 1
  [../]

  #[./iface_x_lr]
  #  type = InterfaceDiffusionBoundaryTerm
  #  boundary = 'left'
  #  variable = disp_x
  #  neighbor_var = disp_x
  #[../]
  [./lambda_yy_lr]
    type = EqualGradientLagrangeMultiplier
    variable = lagrange_e_yy_lr
    boundary = 'left'
    element_var = disp_y
    neighbor_var = disp_y
    component = 1
  [../]
  [./constraint_yy_lr]
    type = EqualGradientLagrangeInterface
    boundary = 'left'
    lambda = lagrange_e_yy_lr
    variable = disp_y
    neighbor_var = disp_y
    component = 1
  [../]


  [./iface_y_tb]
    type = InterfaceDiffusionBoundaryTerm
    boundary = 'top'
    variable = disp_y
    neighbor_var = disp_y
  [../]
  [./lambda_yy_tb]
    type = EqualGradientLagrangeMultiplier
    variable = lagrange_e_yy_tb
    boundary = 'top'
    element_var = disp_y
    neighbor_var = disp_y
    component = 1
  [../]
  [./constraint_yy_tb]
    type = EqualGradientLagrangeInterface
    boundary = 'top'
    lambda = lagrange_e_yy_tb
    variable = disp_y
    neighbor_var = disp_y
    component = 1
  [../]

  #[./iface_y_tb]
  #  type = InterfaceDiffusionBoundaryTerm
  #  boundary = 'top'
  #  variable = disp_y
  #  neighbor_var = disp_y
  #[../]
  [./lambda_yx_tb]
    type = EqualGradientLagrangeMultiplier
    variable = lagrange_e_xy_tb
    boundary = 'top'
    element_var = disp_y
    neighbor_var = disp_y
    component = 0
  [../]
  [./constraint_yx_tb]
    type = EqualGradientLagrangeInterface
    boundary = 'top'
    lambda = lagrange_e_xy_tb
    variable = disp_y
    neighbor_var = disp_y
    component = 0
  [../]

  #[./iface_y_tb]
  #  type = InterfaceDiffusionBoundaryTerm
  #  boundary = 'top'
  #  variable = disp_y
  #  neighbor_var = disp_y
  #[../]
  [./lambda_xx_tb]
    type = EqualGradientLagrangeMultiplier
    variable = lagrange_e_xx_tb
    boundary = 'top'
    element_var = disp_x
    neighbor_var = disp_x
    component = 0
  [../]
  [./constraint_xx_tb]
    type = EqualGradientLagrangeInterface
    boundary = 'top'
    lambda = lagrange_e_xx_tb
    variable = disp_x
    neighbor_var = disp_x
    component = 0
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
      translation = '1 0 0'
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
    petsc_options = '-snes_converged_reason'
    petsc_options_iname = '-ksp_gmres_restart -ksp_rtol -pc_type -pc_factor_shift_type'
    petsc_options_value = '150                1e-4       lu           nonzero'
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2 #does not seem to matter if bdf2 or implicit-euler
  solve_type = 'PJFNK'

  line_search = basic

  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10

  start_time = 0.0
  num_steps = 500

  l_max_its = 35
  nl_max_its = 12
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
  file_base = out_test_lagrange_strain_test
  [./table]
    type = CSV
    delimiter = ' '
  [../]
[]
