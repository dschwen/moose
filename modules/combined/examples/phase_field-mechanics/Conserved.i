#
# Example 1
# Illustrating the coupling between chemical and mechanical (elastic) driving forces.
# An oversized precipitate deforms under a uniaxial compressive stress
# Check the file below for comments and suggestions for parameter modifications.
#

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
  xmax = 50
  ymax = 50
[]

[Variables]
  [./c]
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 0
      y1 = 0
      radius = 25.0
      invalue = 1.0
      outvalue = 0.0
      int_width = 50.0
    [../]
  [../]
  [./w]
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    generate_output = 'stress_xx stress_yy'
    eigenstrain_names = eigenstrain
  [../]
[]

[Kernels]
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
    type = CoupledTimeDerivative
    variable = w
    v = c
  [../]
[]

[Materials]
  [./pfmobility]
    type = GenericConstantMaterial
    prop_names  = 'M kappa_c'
    prop_values = '1 5'
    #kappa = 0.1
    #mob = 1e-3
  [../]

  # simple chemical free energy with a miscibility gap
  [./chemical_free_energy]
    type = DerivativeParsedMaterial
    f_name = Fc
    args = 'c'
    constant_names       = 'barr_height  cv_eq'
    constant_expressions = '0.1          1.0e-2'
    function = 16*barr_height*(c-cv_eq)^2*(1-cv_eq-c)^2
    enable_jit = true
    derivative_order = 2
  [../]

  # undersized solute (voidlike)
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    # lambda, mu values
    C_ijkl = '7 7'
    # Stiffness tensor is created from lambda=7, mu=7 using symmetric_isotropic fill method
    fill_method = symmetric_isotropic
    # See RankFourTensor.h for details on fill methods
    # '15 15' results in a high stiffness (the elastic free energy will dominate)
    # '7 7' results in a low stiffness (the chemical free energy will dominate)
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]
  [./var_dependence]
    type = DerivativeParsedMaterial
    # eigenstrain coefficient
    # -0.1 will result in an undersized precipitate
    #  0.1 will result in an oversized precipitate
    function = 0.1*c
    args = c
    f_name = var_dep
    enable_jit = true
    derivative_order = 2
  [../]
  [./eigenstrain]
    type = ComputeVariableEigenstrain
    eigen_base = '1 1 1 0 0 0'
    prefactor = var_dep
    #outputs = exodus
    args = 'c'
    eigenstrain_name = eigenstrain
  [../]
  [./elastic_free_energy]
    type = ElasticEnergyMaterial
    f_name = Fe
    args = 'c'
    derivative_order = 2
  [../]

  # Sum up chemical and elastic contributions
  [./free_energy]
    type = DerivativeSumMaterial
    f_name = F
    sum_materials = 'Fc Fe'
    args = 'c'
    derivative_order = 2
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

    # prescribed displacement
    # -5 will result in a compressive stress
    #  5 will result in a tensile stress
    value = -5
  [../]
  [./left_x]
    type = PresetBC
    variable = disp_x
    boundary = 'left'
    value = 0
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    coupled_groups = 'c,w disp_x,disp_y'
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2

  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type  -sub_pc_type '
  petsc_options_value = 'asm       lu'

  l_max_its = 30
  nl_max_its = 12
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10
  start_time = 0.0
  end_time = 2000

  [./TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 9
    iteration_window = 2
    growth_factor = 1.75
    cutback_factor = 0.75
    dt = 1
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  print_linear_residuals = false
[]
