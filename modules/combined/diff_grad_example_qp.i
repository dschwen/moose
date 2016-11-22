#
# Diffusion with neighbor relation of quasiperiodicity
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 50
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
[]

[UserObjects]
  [./quasiperiodicneighbors_x]
    type = QuasiPeriodicNeighbors
    execute_on = 'initial linear nonlinear timestep_begin'
    component = 0
  [../]
  [./quasiperiodicneighbors_y]
    type = QuasiPeriodicNeighbors
    execute_on = 'initial linear nonlinear timestep_begin'
    component = 1
  [../]
[]

[GlobalParams]
  penalty = 1.0
  D = 1.00
  D_neighbor = 1.0
  use_displaced_mesh = false
[]

[Variables]
  [./u]
    [./InitialCondition]
      type = FunctionIC
      function = 'r:=(x-0.25)^2+(y-0.25)^2;if(r<0.02,5.0,0)+x'
    [../]
    block = 0
  [../]
  [./v] #add PBC
    [./InitialCondition]
      type = FunctionIC
      function = 'r:=(x-0.25)^2+(y-0.25)^2;if(r<0.02,5.0,0)'
    [../]
    block = 0
  [../]
[]

[AuxVariables]
  [./u_grad_x]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./slope] #sol for u and v +slope should be same
    [./InitialCondition]
      type = FunctionIC
      function = 'x'
    [../]
  [../]
  [./u_grad_y]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./R_u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxKernels]
  [./comp_R_u]
    type = DebugResidualAux
    block = 0
    execute_on = 'timestep_end'
    variable = R_u
    debug_variable = u
  [../]
  [./ugrad_xA]
    type = VariableGradientComponent
    variable = u_grad_x
    gradient_variable = u
    component = x
    execute_on = 'initial timestep_end'
  [../]
  [./ugrad_yA]
    type = VariableGradientComponent
    variable = u_grad_y
    gradient_variable = u
    component = y
    execute_on = 'initial timestep_end'
  [../]
[]

[Kernels]
  [./u1_diff]
    type = Diffusion
    variable = u
  [../]
  [./u1_dt]
    type = TimeDerivative
    variable = u
  [../]

  [./v1_diff]
    type = Diffusion
    variable = v
  [../]
  [./v1_dt]
    type = TimeDerivative
    variable = v
  [../]
[]


[InterfaceKernels]
  [./x_int_left]
    type = QuasiPeriodicGradientTestKernel
    variable = u
    neighbor_var = u
    component = 0
    boundary = 'right'
  [../]
  [./y_int_top]
    type = QuasiPeriodicGradientTestKernel
    variable = u
    neighbor_var = u
    component = 1
    boundary = 'top'
  [../]
[]

[BCs]
 [./Periodic]
   [./up_down]
     primary = top
     secondary = bottom
     translation = '0 -1.0 0'
     variable = 'v'
   [../]
   [./left_right]
     primary = left
     secondary = right
     translation = '1.0 0 0'
     variable = 'v'
   [../]
 [../]

 [./dif_f_l]
   type = DiffusionFluxBC
   variable = u
   boundary = 'left'
 [../]
 [./dif_f_r]
   type = DiffusionFluxBC
   variable = u
   boundary = 'right'
 [../]
 [./dif_f_b]
   type = DiffusionFluxBC
   variable = u
   boundary = 'bottom'
 [../]
 [./dif_f_t]
   type = DiffusionFluxBC
   variable = u
   boundary = 'top'
 [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
  [../]
[]

[Executioner]
  type = Transient
  scheme = implicit-euler #does not seem to matter if bdf2 or implicit-euler
  solve_type = 'PJFNK'

  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10

  start_time = 0.0

  dt = 0.001
  dtmax = 0.001

  num_steps = 20
[]

[Outputs]
  execute_on = 'timestep_end'
  print_linear_residuals = true
  exodus = true
  file_base = out_test_diff_u_periodic_uv_test
  [./table]
    type = CSV
    delimiter = ' '
  [../]
[]
