# Explicitly activate global strain without periodic displacement constraints.

[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    xmin = -0.5
    xmax = 0.5
    ymin = -0.5
    ymax = 0.5
  []
  [center]
    type = ExtraNodesetGenerator
    input = generated_mesh
    coord = '0 0'
    new_boundary = center
  []
[]

[Variables]
  [u_x]
  []
  [u_y]
  []
  [global_strain]
    order = THIRD
    family = SCALAR
  []
[]

[GlobalParams]
  displacements = 'u_x u_y'
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = false
    strain = SMALL
    global_strain = global_strain
  []
[]

[ScalarKernels]
  [global_strain]
    type = GlobalStrain
    variable = global_strain
    global_strain_uo = global_strain_uo
  []
[]

[BCs]
  [center_x]
    type = DirichletBC
    variable = u_x
    boundary = center
    value = 0
  []
  [bottom_y]
    type = DirichletBC
    variable = u_y
    boundary = bottom
    value = 0
  []
  [top_y]
    type = DirichletBC
    variable = u_y
    boundary = top
    value = -0.1
  []
[]

[Materials]
  [elasticity]
    type = ComputeElasticityTensor
    C_ijkl = '1 1'
    fill_method = symmetric_isotropic
  []
  [global_strain]
    type = ComputeGlobalStrain
    scalar_global_strain = global_strain
    global_strain_uo = global_strain_uo
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[UserObjects]
  [global_strain_uo]
    type = GlobalStrainUserObject
    displacements = 'u_x u_y'
    active_directions = x
    execute_on = 'Initial Linear Nonlinear'
  []
[]

[Postprocessors]
  [global_strain_xx]
    type = ScalarVariable
    variable = global_strain
    component = 0
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 1
  dt = 1
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
[]

[Outputs]
  csv = true
[]
