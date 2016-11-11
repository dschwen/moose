#
# Eigenstrain with Mortar gradient periodicity
#

[Mesh]
  type = MortarPeriodicMesh
  dim = 2
  nx = 50
  ny = 50
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  periodic_directions = 'x y'
[]

[Modules/PhaseField/MortarPeriodicity]
  [./strain]
    variable = 'disp_x disp_y'
    periodicity = gradient
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    block = 0
    strain = SMALL
    add_variables = true
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
  derivative_order = 2
  enable_jit = true
  displacements = 'disp_x disp_y'
  op_num = 8 # Number of order parameters used
  var_name_base = gr # Base name of grains
  grain_num = 8 #Number of grains
[]

# AuxVars to compute the free energy density for outputting
[AuxVariables]
  [./local_energy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./local_free_energy]
    type = TotalFreeEnergy
    block = 0
    execute_on = 'initial LINEAR'
    variable = local_energy
    interfacial_vars = 'c'
    kappa_names = 'kappa_c'
  [../]
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./PolycrystalVoronoiIC]
    [../]
  [../]
[]

[Kernels]
  # Custom action creating all necessary kernels for grain growth.  All input parameters are up in GlobalParams
  [./PolycrystalKernel]
  [../]
[]

[Materials]
  [./CuGrGr]
    # Material properties
    type = GBEvolution # Quantitative material properties for copper grain growth.  Dimensions are nm and ns
    block = 0 # Block ID (only one block in this problem)
    GBmob0 = 2.5e-6 #Mobility prefactor for Cu from Schonfelder1997
    GBenergy = 0.708 # GB energy in J/m^2
    Q = 0.23 #Activation energy for grain growth from Schonfelder 1997
    T = 450 # K   #Constant temperature of the simulation (for mobility calculation)
    wGB = 14 # nm    #Width of the diffuse GB
    outputs = exodus
  [../]

  [./elasticity]
    type = ComputeElasticityTensor
    C_ijkl = '1 1'
    fill_method = symmetric_isotropic
  [../]


[]

[BCs]
  [./Periodic]
    [./up_down]
      primary = top
      secondary = bottom
      translation = '0 -1 0'
      variable = 'gr0 gr1 gr2 gr3 gr4 gr5 gr6 gr7'
    [../]
    [./left_right]
      primary = left
      secondary = right
      translation = '1 0 0'
      variable = 'gr0 gr1 gr2 gr3 gr4 gr5 gr6 gr7'
    [../]
  [../]

  # fix center point location
  [./centerfix_x]
    type = PresetBC
    boundary = 100
    variable = disp_x
    value = 0
  [../]
  [./centerfix_y]
    type = PresetBC
    boundary = 100
    variable = disp_y
    value = 0
  [../]

  # fix side point x coordinate to inhibit rotation
  [./angularfix]
    type = PresetBC
    boundary = 101
    variable = disp_x
    value = 0
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[UserObjects]
  [./grain_tracker]
    type = GrainTrackerRankTwo
    threshold = 0.2
    connecting_threshold = 0.08
    flood_entity_type = ELEMENTAL

    base_tensor = '0.1 0 0 0 0 0'
    random_rotations = true
  [../]
  [./euler_angle_file]
    type = EulerAngleFileReader
    file_name = grn_8_rand_2D.tex
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = 'PJFNK'

  line_search = basic

  #petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart -mat_mffd_type'
  #petsc_options_value = 'hypre boomeramg 101 ds'

  # mortar currently does not support MPI parallelization
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = ' lu       NONZERO               1e-10'

  l_max_its = 30
  nl_max_its = 12

  l_tol = 1.0e-4

  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10

  start_time = 0.0
  num_steps = 200

  [./TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 0.01
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  print_linear_residuals = false
  exodus = true
  [./table]
    type = CSV
    delimiter = ' '
  [../]
[]
