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

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = LatticeSmoothCircleIC
      block = 0
      Rnd_variation = 5.0
      circles_per_side = '12 12'
      radius = 5
      int_width = 4
      invalue = 0.84
      outvalue = 0.16
      avoid_bounds = false
    [../]
  [../]
  [./w]
    order = FIRST
    family = LAGRANGE
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
  [./time]
    type = CoupledImplicitEuler
    variable = w
    v = c
  [../]
[]

#
# The AuxVariables and AuxKernels below are added to visualize the xx and yy stress tensor components
#
[AuxVariables]
  [./e11_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e22_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]
[AuxKernels]
  [./matl_e11]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
    variable = e11_aux
  [../]
  [./matl_e22]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 1
    index_j = 1
    variable = e22_aux
  [../]
[]

[Materials]
  [./pfmobility]
    type = PFMobility
    block = 0
    #kappa = 0.1
    kappa = 0.5
    #mob = 1e-3
    mob = 1
  [../]

  # simple chemical free energy with a miscibility gap
  [./chemical_free_energy]
    type = DerivativeParsedMaterial
    block = 0
    f_name = Fc
    args = 'c'
    constant_names       = 'barr_height  cv_eq'
    constant_expressions = '0.1          0'
    function = '16*barr_height*(c-cv_eq)^2*(1-cv_eq-c)^2 - 0.02*(c*plog(c,1e-3)+(1-c)*plog(1-c,1e-3))'
    enable_jit = true
    third_derivatives = false
  [../]

  # undersized solute (voidlike)
  [./eigenstrain]
    type = SimpleEigenStrainMaterial
    block = 0

    # eigenstrain coefficient
    # -0.1 will result in an undersized precipitate
    #  0.1 will result in an oversized precipitate
    epsilon0 = -0.1

    c = c
    disp_y = disp_y
    disp_x = disp_x

    # Stiffness tensor lambda, mu values
    # '15 15' results in a high stiffness (the elastic free energy will dominate)
    # '7 7' results in a low stiffness (the chemical free energy will dominate)
    C_ijkl = '15 5'

    fill_method = symmetric_isotropic
  [../]
  [./elastic_free_energy]
    type = ElasticEnergyMaterial
    f_name = Fe
    block = 0
    args = 'c'
    third_derivatives = false
  [../]

  # Sum up chemical and elastic contributions
  [./free_energy]
    type = DerivativeSumMaterial
    block = 0
    f_name = F
    sum_materials = 'Fc Fe'
    args = 'c'
    third_derivatives = false
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
  [../]
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
  #petsc_options_iname = '-pc_type -ksp_grmres_restart -sub_ksp_type -sub_pc_type -pc_asm_overlap'
  #petsc_options_value = 'asm         101   preonly   lu      1'

  l_max_its = 50
  nl_max_its = 50
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10
  start_time = 0.0
  num_steps = 800

  [./TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 0.01
  [../]
[]

[Outputs]
  interval = 1
  exodus = true
  output_on = timestep_end
  [./console]
    type = Console
    perf_log = true
    output_on = 'timestep_end failed nonlinear linear'
  [../]
[]
