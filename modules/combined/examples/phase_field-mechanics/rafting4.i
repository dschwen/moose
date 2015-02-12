#
# Example 1
# Illustrating the coupling between chemical and mechanical (elastic) driving forces.
# An oversized precipitate deforms under a uniaxial compressive stress
# Check the file below for comments and suggestions for parameter modifications.
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 100
  nz = 0
  xmin = 0
  xmax = 160
  ymin = 0
  ymax = 160
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[Functions]
  [./tween]
    type = ParsedFunction
    value = 'if(t<10.0,max((t-0.2)/10.0,0.01),1)'
  [../]
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = LatticeSmoothCircleIC
      block = 0
      Rnd_variation = 5.0
      circles_per_side = '10 10'
      radius = 5
      int_width = 4
      invalue = 0.8
      outvalue = 0.2
      avoid_bounds = false
    [../]
  [../]
  [./w]
    order = FIRST
    family = LAGRANGE
  [../]
  [./eta]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.5
  [../]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./TensorMechanics]
    disp_x = disp_x
    disp_y = disp_y
  [../]

  [./eta_res]
    type = ACParsed
    variable = eta
    args = 'c'
    f_name = F
  [../]
  [./eta_int]
    type = ACInterface
    variable = eta
    kappa_name = kappa_op
  [../]

  [./c_res]
    type = SplitCHParsed
    variable = c
    f_name = F
    kappa_name = kappa_c
    args = 'eta'
    w = w
  [../]
  [./w_res]
    type = SplitCHWRes
    variable = w
    mob_name = M
  [../]

    [./time_c]
    type = CoupledImplicitEuler
    variable = w
    v = c
  [../]
  [./time_eta]
    type = TimeDerivative
    variable = eta
  [../]
[]

#
# The AuxVariables and AuxKernels below are added to visualize the xx and yy stress tensor components
#
# [AuxVariables]
#   [./e11_aux]
#     order = CONSTANT
#     family = MONOMIAL
#   [../]
#   [./e22_aux]
#     order = CONSTANT
#     family = MONOMIAL
#   [../]
# []
# [AuxKernels]
#   [./matl_e11]
#     type = RankTwoAux
#     rank_two_tensor = stress
#     index_i = 0
#     index_j = 0
#     variable = e11_aux
#   [../]
#   [./matl_e22]
#     type = RankTwoAux
#     rank_two_tensor = stress
#     index_i = 1
#     index_j = 1
#     variable = e22_aux
#   [../]
# []

[Materials]
  [./const]
    type = GenericConstantMaterial
    prop_names =  'kappa_op  kappa_c  L  M'
    prop_values = '3         2        1  1'
    block = 0
  [../]

  [./switching]
    type = SwitchingFunctionMaterial
    block = 0
    eta = eta
    # h_order = SIMPLE
  [../]
  [./barrier]
    type = BarrierFunctionMaterial
    block = 0
    eta = eta
    # g_order = SIMPLE
  [../]

  # chemical free energy
  [./chemical_free_energy_1]
    type = DerivativeParsedMaterial
    block = 0
    f_name = Fc1
    args = 'c'
    function = 'c^2'
    enable_jit = true
    third_derivatives = false
    outputs = exodus
  [../]

  # chemical free energy
  [./chemical_free_energy_2]
    type = DerivativeParsedMaterial
    block = 0
    f_name = Fc2
    args = 'c'
    function = '(1-c)^2'
    enable_jit = true
    third_derivatives = false
    outputs = exodus
  [../]

  # hard matrix
  [./material_1]
    type = LinearElasticMaterial
    block = 0
    base_name = phase1
    disp_y = disp_y
    disp_x = disp_x
    C_ijkl = '100 100'
    elasticity_tensor_prefactor = tween
    fill_method = symmetric_isotropic
  [../]

  # soft precipitate
  [./material_2]
    type = LinearElasticMaterial
    block = 0
    base_name = phase2
    disp_y = disp_y
    disp_x = disp_x
    C_ijkl = '1 1'
    fill_method = symmetric_isotropic
  [../]

  [./elastic_free_energy_1]
    type = ElasticEnergyMaterial
    f_name = Fe1
    base_name = phase1
    block = 0
    args = 'c'
    third_derivatives = false
  [../]
  [./elastic_free_energy_2]
    type = ElasticEnergyMaterial
    f_name = Fe2
    base_name = phase2
    block = 0
    args = 'c'
    third_derivatives = false
  [../]

  # Sum up chemical and elastic contributions
  [./free_energy_1]
    type = DerivativeSumMaterial
    block = 0
    f_name = F1
    sum_materials = 'Fc1 Fe1'
    args = 'c'
    third_derivatives = false
  [../]
  [./free_energy_2]
    type = DerivativeSumMaterial
    block = 0
    f_name = F2
    sum_materials = 'Fc2 Fe2'
    args = 'c'
    third_derivatives = false
  [../]

  # Two phase
  [./free_energy]
    type = DerivativeTwoPhaseMaterial
    block = 0
    f_name = F
    fa_name = F1
    fb_name = F2
    args = 'c'
    eta = eta
    W = 0.1
    third_derivatives = false
    outputs = exodus
  [../]

  # Two phase, just chemical (for debugging)
  [./total_chemical_free_energy]
    type = DerivativeTwoPhaseMaterial
    block = 0
    f_name = Fc
    fa_name = Fc1
    fb_name = Fc2
    args = 'c'
    eta = eta
    third_derivatives = false
    outputs = exodus
  [../]
  # Two phase, just elastic (for debugging)
  [./total_elastic_free_energy]
    type = DerivativeTwoPhaseMaterial
    block = 0
    f_name = Fe
    fa_name = Fe1
    fb_name = Fe2
    args = 'c'
    eta = eta
    third_derivatives = false
    outputs = exodus
  [../]

  [./global_stress]
    type = TwoPhaseStressMaterial
    block = 0
    base_A = phase1
    base_B = phase2
  [../]
[]

[BCs]
  [./bottom_y]
    type = PresetBC
    variable = disp_y
    boundary = 'bottom'
    value = 0
  [../]
  [./top_y]
    type = PresetBC
    variable = disp_y
    boundary = 'top'
    value = 16
  [../]
  [./left_x]
    type = PresetBC
    variable = disp_x
    boundary = 'left'
    value = 0
  [../]
  [./right_x]
    type = PresetBC
    variable = disp_x
    boundary = 'right'
    value = 0
  [../]

  [./Periodic]
    [./c]
      variable = c
      auto_direction = 'x y'
    [../]
    [./w]
      variable = w
      auto_direction = 'x y'
    [../]
    [./eta]
      variable = eta
      auto_direction = 'x y'
    [../]
  [../]
[]

[Postprocessors]
  [./tween_plot]
    type = PlotFunction
    function = tween
  []
[]

[Preconditioning]
  # active = ' '
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2

  solve_type = 'PJFNK'
  # petsc_options_iname = '-pc_type -ksp_grmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
  # petsc_options_value = 'asm         101   preonly   lu      1'

  l_max_its = 30
  nl_max_its = 50
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-6
  nl_abs_tol = 1.0e-8
  start_time = 0.0
  num_steps = 800

  [./TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 0.2
  [../]
[]

[Debug]
  # show_var_residual_norms = true
[]

[Outputs]
  interval = 1
  exodus = true
  output_on = timestep_end
  [./table]
    type = CSV
  [../]
  [./console]
    type = Console
    perf_log = true
    output_on = 'initial timestep_begin timestep_end failed nonlinear'
  [../]
[]
