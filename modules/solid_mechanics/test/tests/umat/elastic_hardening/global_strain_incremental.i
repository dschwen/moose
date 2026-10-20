# Exercise an incremental UMAT with explicit global strain and no periodic BCs.

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    xmin = -0.5
    xmax = 0.5
    ymin = -0.5
    ymax = 0.5
    zmin = -0.5
    zmax = 0.5
  []
  [center]
    type = ExtraNodesetGenerator
    input = gen
    coord = '0 0 0'
    new_boundary = center
    use_closest_node = true
  []
[]

[Variables]
  [global_strain]
    order = SIXTH
    family = SCALAR
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    add_variables = true
    strain = FINITE
    incremental = true
    global_strain = global_strain
    generate_output = 'stress_xx strain_xx'
  []
[]

[Physics/SolidMechanics/GlobalStrain]
  [prp]
    scalar_global_strain = global_strain
    displacements = 'disp_x disp_y disp_z'
    auxiliary_displacements = 'total_disp_x total_disp_y total_disp_z'
    global_displacements = 'global_disp_x global_disp_y global_disp_z'
    active_directions = x
  []
[]

[BCs]
  [center_x]
    type = DirichletBC
    variable = disp_x
    boundary = center
    value = 0
  []
  [bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [top_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = top
    function = '-0.1*t'
  []
  [back_z]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  []
[]

[Materials]
  [umat]
    type = AbaqusUMATStress
    constant_properties = '1000 0.3'
    plugin = '../../../plugins/elastic_incremental'
    num_state_vars = 0
    use_one_based_indexing = true
  []
[]

[Postprocessors]
  [global_strain_xx]
    type = ScalarVariable
    variable = global_strain
    component = 0
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  start_time = 0
  dt = 1
  num_steps = 1
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
