[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 8
    xmin = 0.0
    xmax = 1.0
    ymin = 0.0
    ymax = 2.0
    elem_type = QUAD4
  []
  [rotate]
    type = TransformGenerator
    transform = ROTATE
    vector_value = '0 0 -60'
    input = generated_mesh
  []
  [lower]
    type = LowerDBlockFromSidesetGenerator
    input = rotate
    sidesets = 'right bottom'
    new_block_name = lower
  []
[]

[Variables]
  [lambda]
    order = CONSTANT
    family = MONOMIAL
    block = lower
  []
[]

[Kernels]
  [lambda]
    type = LagrangeInclinedNoDisplacement
    variable = lambda
    block = lower
  []
[]

[Modules/TensorMechanics/Master/All]
  strain = FINITE
  add_variables = true
[]

[BCs]
  [Pressure]
    [top]
      boundary = top
      function = '-1000*t'
    []
  []

  [right_x]
    type = LagrangeInclinedNoDisplacementBC
    variable = disp_x
    component = 0
    boundary = right
    lambda = lambda
  []
  [right_y]
    type = LagrangeInclinedNoDisplacementBC
    variable = disp_y
    component = 1
    boundary = right
    lambda = lambda
  []

  [bottom_x]
    type = LagrangeInclinedNoDisplacementBC
    variable = disp_x
    component = 0
    boundary = bottom
    lambda = lambda
  []
  [bottom_y]
    type = LagrangeInclinedNoDisplacementBC
    variable = disp_y
    component = 1
    boundary = bottom
    lambda = lambda
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1e6
    poissons_ratio = 0.3
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
  []
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package -pc_factor_shift_type'
  petsc_options_value = 'lu     superlu_dist nonzero'

  # controls for linear iterations
  l_max_its = 10
  l_tol = 1e-4

  # controls for nonlinear iterations
  nl_max_its = 100
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8

  # time control
  start_time = 0.0
  dt = 1
  end_time = 5
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Outputs]
  exodus = true
  execute_on = 'NONLINEAR TIMESTEP_END'
[]
