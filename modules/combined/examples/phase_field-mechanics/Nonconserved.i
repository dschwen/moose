#
# Example 2
# Phase change driven by a mechanical (elastic) driving force.
# An oversized phase inclusion grows under a uniaxial tensile stress.
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
  xmin = 0
  xmax = 50
  ymax = 50
[]

[Variables]
  [./eta]
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
[]

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    generate_output = 'stress_xx stress_yy'
    eigenstrain_names = eigenstrain
  [../]
[]

[Kernels]
  [./eta_bulk]
    type = AllenCahn
    variable = eta
    f_name = F
  [../]
  [./eta_interface]
    type = ACInterface
    variable = eta
    kappa_name = kappa
  [../]
  [./time]
    type = TimeDerivative
    variable = eta
  [../]
[]

[AuxVariables]
  [./F]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./F]
    type = TotalFreeEnergy
    variable = F
    kappa_names = kappa
    interfacial_vars = eta
    f_name = F
    execute_on = 'timestep_end initial'
  [../]
[]

[Materials]
  [./consts]
    type = GenericConstantMaterial
    prop_names  = 'L kappa'
    prop_values = '1 0.1'
  [../]

  # matrix phase
  [./stiffness]
    type = ComputeElasticityTensor
    # lambda, mu values
    C_ijkl = '7 7'
    # Stiffness tensor is created from lambda=7, mu=7 for symmetric_isotropic fill method
    fill_method = symmetric_isotropic
    # See RankFourTensor.h for details on fill methods
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]

  [./phase_mixture_energy]
    type = BarrierFunctionMaterial
    eta = eta
    outputs = exodus
  [../]
  [./elastic_free_energy]
    type = ElasticEnergyMaterial
    f_name = Fe
    args = eta
    outputs = exodus
  [../]

  [./free_energy]
    type = DerivativeSumMaterial
    sum_materials = 'Fe g'
    prefactor     = '1  0.2'
    args = eta
  [../]

  [./switching]
    type = SwitchingFunctionMaterial
    eta = eta
  [../]
  [./eigenstrain]
    type = ComputeVariableEigenstrain
    eigen_base = '0.05 0.2 0.0'
    eigenstrain_name = eigenstrain
    # scale Eigenstrain with a switching function
    prefactor = h
    args = eta
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
    value = 5
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
    coupled_groups = 'disp_x,disp_y'
  [../]
[]

[Postprocessors]
  [./F]
    type = ElementIntegralVariablePostprocessor
    variable = F
    execute_on = 'timestep_end initial'
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2

  # this gives best performance on 4 cores
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type  -sub_pc_type '
  petsc_options_value = 'asm       lu'

  l_max_its = 30
  nl_max_its = 12
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10
  start_time = 0.0
  end_time = 1500

  [./TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 9
    iteration_window = 2
    growth_factor = 1.75
    cutback_factor = 0.75
    dt = 0.3
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  print_linear_residuals = false
  exodus = true
  csv = true
[]
