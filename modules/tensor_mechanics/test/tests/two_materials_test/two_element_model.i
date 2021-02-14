[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

[Mesh]
  file = gold/TwoElementModel.e
  construct_side_list_from_node_list = true
  # block 1 add hex 1
  # block 2 add hex 2
  #
  # nodeset 1 add vertex 4 # front left  bottom
  # nodeset 2 add vertex 7 # back  left  bottom
  # nodeset 3 add vertex 8 # back  left  top
  # nodeset 4 add vertex 3 # front left  top
  # nodeset 5 add vertex 1 # front right bottom
  # nodeset 6 add vertex 6 # back  right bottom
  # nodeset 7 add vertex 5 # back  right top
  # nodeset 8 add vertex 2 # front right top
[]

[AuxVariables]
 [./resid_x]
 [../]
 [./resid_y]
 [../]
 [./resid_z]
 [../]
[]

[Modules/TensorMechanics/Master]
 generate_output = 'stress_xx stress_yy stress_zz stress_xy stress_yz stress_zx vonmises_stress hydrostatic_stress elastic_strain_xx elastic_strain_yy elastic_strain_zz strain_xx strain_yy strain_zz'
 [./concrete]
   block = '1'
   strain = FINITE
   add_variables = true
   save_in = 'resid_x resid_y resid_z'
 [../]
 [./soil]
   block = '2'
   strain = FINITE
   save_in = 'resid_x resid_y resid_z'
 [../]
[]

[Materials]
  [./creep]
   type = LinearViscoelasticStressUpdate
   block = '1'
  [../]
  [burgers]
    type = GeneralizedKelvinVoigtModel
    creep_modulus = '1.52e12
                    5.32e18
                    6.15e10
                    6.86e10
                    4.48e10
                    1.05e128'
    creep_viscosity = '1
                      10
                      100
                      1000
                      10000
                      100000'
    poisson_ratio = 0.2
    young_modulus = 27.8e9
    block = 1
  []
  [./stress_concrete]
    type = ComputeMultipleInelasticStress
    block = '1'
    inelastic_models = 'creep'
  [../]

  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = '2'
    fill_method = symmetric_isotropic
    C_ijkl = '0 5E9'
  [../]
  [./mc]
    type = ComputeMultiPlasticityStress
    block = '2'
    ep_plastic_tolerance = 1E-11
    plastic_models = mc
    max_NR_iterations = 1000
    debug_fspb = crash
  [../]
[]

[UserObjects]
  [./visco_update]
    type = LinearViscoelasticityManager
    block = '1'
    viscoelastic_model = burgers
  [../]
  [./mc_coh]
    type = TensorMechanicsHardeningConstant
    block = '2'
    value = 10E6
  [../]
  [./mc_phi]
    type = TensorMechanicsHardeningConstant
    block = '2'
    value = 40
    convert_to_radians = true
  [../]
  [./mc_psi]
    type = TensorMechanicsHardeningConstant
    block = '2'
    value = 40
    convert_to_radians = true
  [../]
  [./mc]
    type = TensorMechanicsPlasticMohrCoulomb
    block = '2'
    cohesion = mc_coh
    friction_angle = mc_phi
    dilation_angle = mc_psi
    mc_tip_smoother = 0.01E6
    mc_edge_smoother = 29
    yield_function_tolerance = 1E-5
    internal_constraint_tolerance = 1E-11
  [../]
[]

[BCs]
 [./x_disp]
   type = DirichletBC
   variable = disp_x
   boundary = '1 3'
   value    = 0.0
 [../]
 [./y_disp]
   type = DirichletBC
   variable = disp_y
   boundary = '1 2'
   value    = 0.0
 [../]
 [./z_disp]
   type = DirichletBC
   variable = disp_z
   boundary = '1 5'
   value    = 0.0
 [../]
[]

[Executioner]
  type       = Transient
  start_time = 10
  dt = 1
  automatic_scaling = true
  end_time = 100

  # working solver conditions
  solve_type = 'NEWTON'
  nl_max_its = 100
  l_max_its = 100
  nl_abs_tol = 1.E-5
  nl_rel_tol = 1E-3
  line_search = none
  petsc_options = '-ksp_snes_ew'
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart -snes_ls -pc_hypre_boomeramg_strong_threshold'
  petsc_options_value = 'hypre boomeramg 201 cubic 0.7'
[]

[Outputs]
 perf_graph = true
 csv = true
 [./Console]
   type = Console
 [../]
 [./Exo]
   type = Exodus
   elemental_as_nodal = true
 [../]
[]
