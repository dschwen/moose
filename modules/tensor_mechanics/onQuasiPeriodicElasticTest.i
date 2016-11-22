
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 5
  ny = 5
  nz = 5
  ymax = 3
  ymin = -3
  xmin = -3
  xmax = 3
  zmin = -3
  zmax = 3
  # DO NOT USE AMR.
  # DO NOT USE UNIFORM REFINE.
  # DO NOT PASS GO.
  # DO NOT COLLECT $200.
  uniform_refine = 0
[]

[UserObjects]
  # Note that this doesn't work for MPI in combination with elasticity problems.
  # The strange that thing is that it works for the diffusion problem however.
  # One can see that it isn't this UserObject that is causing the problem
  # but the RVEAntiperiodicTraction object
  [./quasiperiodicneighbors]
    type = QuasiPeriodicNeighbors
    execute_on = 'initial linear nonlinear'
    auto_direction = 'x'
  [../]
[]

[GlobalParams]
  disp_x = disp_x
  disp_y = disp_y
  disp_z = disp_z
  use_displaced_mesh = false
  displacements = 'disp_x disp_y disp_z'
[]


[Functions]
  active = 'ff_1'

  [./ff_1]
    type = ParsedFunction
    value = alpha*exp(-x*x*b-y*y*b-b-2*x*b)
    vars = 'alpha b'
    vals = '-0.05 0.75'
  [../]
[]

[Variables]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
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
[]

[AuxKernels]
  [./matl_e11]
    type = RankTwoAux
    rank_two_tensor = elastic_strain
    index_i = 0
    index_j = 0
    variable = strain_xx_elastic
  [../]
  [./matl_e12]
    type = RankTwoAux
    rank_two_tensor = elastic_strain
    index_i = 0
    index_j = 1
    variable = strain_xy_elastic
  [../]
  [./matl_e13]
    type = RankTwoAux
    rank_two_tensor = elastic_strain
    index_i = 0
    index_j = 2
    variable = strain_xz_elastic
  [../]
  [./matl_e22]
    type = RankTwoAux
    rank_two_tensor = elastic_strain
    index_i = 1
    index_j = 1
    variable = strain_yy_elastic
  [../]
  [./matl_e23]
    type = RankTwoAux
    rank_two_tensor = elastic_strain
    index_i = 1
    index_j = 2
    variable = strain_yz_elastic
  [../]
  [./matl_e33]
    type = RankTwoAux
    rank_two_tensor = elastic_strain
    index_i = 2
    index_j = 2
    variable = strain_zz_elastic
  [../]
  [./matl_s11]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
    variable = stress_xx_elastic
    execute_on = 'timestep_end'
  [../]
  [./matl_s12]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 1
    variable = stress_xy_elastic
    execute_on = 'timestep_end'
  [../]
  [./matl_s13]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 2
    variable = stress_xz_elastic
    execute_on = 'timestep_end'
  [../]
 [./matl_s22]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 1
    index_j = 1
    variable = stress_yy_elastic
    execute_on = 'timestep_end'
  [../]
  [./matl_s23]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 1
    index_j = 2
    variable = stress_yz_elastic
    execute_on = 'timestep_end'
  [../]
  [./matl_s33]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 2
    index_j = 2
    variable = stress_zz_elastic
    execute_on = 'timestep_end'
  [../]
[]

[Materials]
  #[./eigen_strain_zz] #Use for stress-free strain (ie epitaxial)
  #  type = ComputeEigenstrain
  #  # eigen_base = 'exx exy exz eyx eyy eyz ezx ezy ezz'
  #  eigen_base = '1 0 0 0 1 0 0 0 0'
  #[../]

  [./elasticity_tensor_1]
    type = ComputeElasticityTensor
    fill_method = symmetric9
    C_ijkl = '380. 150. 150. 380. 150. 380. 110. 110. 110.'
  [../]
  [./strain_1]
    type = ComputeSmallStrain
  [../]
  [./stress_1]
    type = ComputeLinearElasticStress
  [../]
[]

[Kernels]
  [./TensorMechanics]
  [../]
[]

[InterfaceKernels]
  [./xx_int_left]
    type = RVEAntiperiodicTraction
    variable = disp_x
    neighbor_var = disp_x
    component = 0
    boundary = 'left'
  [../]

  #[./xx_int_right]
  #  type = RVEAntiperiodicTraction
  #  variable = disp_x
  #  neighbor_var = disp_x
  #  component = 0
  #  boundary = 'right'
  #[../]

#  [./yy_int_top]
#    type = RVEAntiperiodicTraction
#    variable = disp_y
#    neighbor_var = disp_y
#    component = 1
#    boundary = 'top'
#  [../]
#
#[./yy_int_bot]
#  type = RVEAntiperiodicTraction
#  variable = disp_y
#  neighbor_var = disp_y
#  component = 1
#  boundary = 'bottom'
#[../]
[]

[BCs]
  [./uztop]
    type = FunctionDirichletBC
    function = ff_1
    variable = disp_z
    boundary = 'front'
  [../]

  #[./uxbottom]
  #  type = DirichletBC
  #  variable = disp_x
  #  boundary = 'back'
  #  value = 0.0
  #[../]
  #[./uybottom]
  #  type = DirichletBC
  #  variable = disp_y
  #  boundary = 'back'
  #  value = 0.0
  #[../]
  [./uzbottom]
    type = DirichletBC
    variable = disp_z
    boundary = 'back'
    value = 0.0
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
    petsc_options = '-snes_view -snes_linesearch_monitor -snes_converged_reason -ksp_converged_reason'
    petsc_options_iname = '-ksp_gmres_restart  -snes_rtol -ksp_rtol -pc_type -pc_hypre_type '
    petsc_options_value = '   480             1e-8      1e-8          hypre      boomeramg  '
  [../]
[]

[Executioner]
  type = Steady
  #l_max_its = 1
  #nl_max_its = 1
  solve_type = 'PJFNK'       #"PJFNK, JFNK, NEWTON"
[]

[Outputs]
  print_linear_residuals = true
  print_perf_log = true
  [./out]
    type = Exodus
    file_base = out_test_with_interface
    elemental_as_nodal = true
  [../]
[]
