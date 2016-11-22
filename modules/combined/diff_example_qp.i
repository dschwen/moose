#
# Eigenstrain with neighbor relation of quasiperiodicity
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

[MeshModifiers]
  [./cnode]
    type = AddExtraNodeset
    coord = '0.3 0.3'
    new_boundary = 100
  [../]

  [./anode]
    type = AddExtraNodeset
    coord = '-0.4 -0.4'
    new_boundary = 101
  [../]
[]

#[Functions]
#  active = 'ff_1'
#  [./ff_1]
#    type = ParsedFunction
#    value = alpha*exp(-x*x*b-y*y*b-b-2*x*b)
#    vars = 'alpha b'
#    vals = '-1.0 7.5'
#  [../]
#[]

[GlobalParams]
  penalty = 1000.0 #~N needed to converge to periodic solution
  D = 1.0
  D_neighbor = 1.0
  use_displaced_mesh = false
[]

[Variables]
  [./u]
    [./InitialCondition]
      type = FunctionIC
      function = 'r:=(x-0.3)^2+(y-0.3)^2;if(r<0.1,5.0,0)'
    [../]
    block = 0
  [../]
[]

[AuxVariables]
  [./u_grad_x]
    order = CONSTANT
    family = MONOMIAL
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
[]

#
[InterfaceKernels]
  [./xx_int_left]
    type = QuasiPeriodicTestKernel
    variable = u
    neighbor_var = u
    component = 0
    boundary = 'left'
  [../]

  ## -----top-----
  [./yy_int_top]
    type = QuasiPeriodicTestKernel
    variable = u
    neighbor_var = u
    component = 0
    boundary = 'top'
  [../]
[]

[BCs]
 #[./Periodic]
 #  [./up_down]
 #    primary = top
 #    secondary = bottom
 #    translation = '0 -1.0 0'
 #    variable = 'u'
 #  [../]
 #  [./left_right]
 #    primary = left
 #    secondary = right
 #    translation = '1.0 0 0'
 #    variable = 'u'
 #  [../]
 #[../]

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

 #[./dirich_au_cent]
 #  type = DirichletBC
 #  variable = u
 #  value = 1.0
 #  boundary = '101'
 #[../]
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
  scheme = bdf2 #does not seem to matter if bdf2 or implicit-euler
  solve_type = 'PJFNK'

  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10

  start_time = 0.0

  dt = 0.01
  dtmax = 0.01

  num_steps = 10
[]

[Outputs]
  execute_on = 'timestep_end'
  print_linear_residuals = true
  exodus = true
  file_base = out_test_diff_u_int_pen
  [./table]
    type = CSV
    delimiter = ' '
  [../]
[]
