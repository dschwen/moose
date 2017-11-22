#
# Example 2
# Phase change driven by a mechanical (elastic) driving force.
# An oversized phase inclusion grows under a uniaxial tensile stress.
# Check the file below for comments and suggestions for parameter modifications.
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
  nz = 0
  xmin = 0
  xmax = 50
  ymin = 0
  ymax = 50
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./eta]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 0
      y1 = 0
      radius = 30.0
      invalue = 1.0
      outvalue = 0.0
      int_width = 10.0
    [../]
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
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
    kappa_name = 1
  [../]
  [./time]
    type = TimeDerivative
    variable = eta
  [../]
[]

#
# Try visualizing the stress tensor components as done in Conserved.i
#

[Materials]
  [./consts]
    type = GenericConstantMaterial
    prop_names  = 'L'
    prop_values = '1'
  [../]

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
  [./eigenstrain]
    type = ComputeVariableEigenstrain
    eigen_base = '0.1 0.1 0.1'
    prefactor = h
    eigenstrain_name = eigenstrain
    args = eta
  [../]
  [./elastic_free_energy]
    type = ElasticEnergyMaterial
    f_name = Fe
    args = eta
  [../]

  # Generate the global free energy from the phase free energies
  [./switching]
    type = SwitchingFunctionMaterial
    eta = eta
    h_order = SIMPLE
  [../]
  [./barrier]
    type = BarrierFunctionMaterial
    eta = eta
    g_order = SIMPLE
  [../]
  [./free_energy]
    type = DerivativeSumMaterial
    sum_materials = 'Fe g'
    f_name = F
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
  # active = ' '
  [./SMP]
    type = SMP
    full = true
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
  nl_max_its = 10
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10
  start_time = 0.0
  num_steps = 200

  [./TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 0.2
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
