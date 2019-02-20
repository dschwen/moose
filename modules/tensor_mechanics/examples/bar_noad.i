[Mesh]
  type = GeneratedMesh
  dim = 2
  xmax = 10
  ymax = 1
  nx = 40
  ny = 4
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[MeshModifiers]
  [./sideset]
    type = ParsedAddSideset
    combinatorial_geometry = 'x >= 8 & y >= 1'
    included_subdomain_ids = 0
    normal = '0 1 0'
    new_sideset_name = pressure
    variance = 0.001
  [../]
[]

[Functions]
  [./pressure]
    type = ParsedFunction
    value = t*1e-5
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[Kernels]
  [./x]
    type = StressDivergenceTensors
    variable = disp_x
    component = 0
    use_displaced_mesh = true
  [../]
  [./y]
    type = StressDivergenceTensors
    variable = disp_y
    component = 1
    use_displaced_mesh = true
  [../]
[]

[AuxVariables]
  [./mp]
    family = MONOMIAL
    order = CONSTANT
  [../]
  [./vm]
    family = MONOMIAL
    order = CONSTANT
  [../]
[]

[AuxKernels]
  [./mp]
    type = RankTwoScalarAux
    variable = mp
    rank_two_tensor = stress
    scalar_type = MaxPrincipal
    execute_on = 'TIMESTEP_END'
  [../]
  [./vm]
    type = RankTwoScalarAux
    variable = vm
    rank_two_tensor = stress
    scalar_type = VonMisesStress
    execute_on = 'TIMESTEP_END'
  [../]
[]

[BCs]
  [./fix_x]
    type = PresetBC
    variable = disp_x
    boundary = 'left'
    value = 0
  [../]
  [./fix_y]
    type = PresetBC
    variable = disp_y
    boundary = 'left'
    value = 0
  [../]
  [./pressure_x]
    type = Pressure
    component = 0
    variable = disp_x
    boundary = 'pressure'
    function = pressure
  [../]
  [./pressure_y]
    type = Pressure
    component = 1
    variable = disp_y
    boundary = 'pressure'
    function = pressure
  [../]
[]

[Materials]
  [./elasticity]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1
    poissons_ratio = 0.3
  [../]
  [./strain]
    type = ComputeFiniteStrain
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
  [../]
[]

[Postprocessors]
  [./time]
    type = PerfGraphData
    data_type = TOTAL
    section_name = Root
  [../]
  [./nl]
    type = NumNonlinearIterations
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  nl_abs_tol = 1e-12
  end_time = 1500
  dtmin = 1

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  [./TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 7
    linear_iteration_ratio = 1000
    iteration_window = 2
    dt = 1
  [../]
  [./Predictor]
    type = AdamsPredictor
    scale = 0.5
  [../]
[../]

[Outputs]
  exodus = true
  csv = true
  print_linear_residuals = true
  perf_graph = true
[]
