
[Mesh]
  type = GeneratedMesh
  dim = 1

  nx = 50

  xmin = -3
  xmax = 3

  # DO NOT USE AMR.
  # DO NOT USE UNIFORM REFINE.
  # DO NOT PASS GO.
  # DO NOT COLLECT $200.
  uniform_refine = 0
[]

[GlobalParams]
  use_displaced_mesh = false
[]

[UserObjects]
  [./quasiperiodicneighbors]
    type = QuasiPeriodicNeighbors
    execute_on = 'initial linear nonlinear'
    #auto_direction = 'x'
    component = 0
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      #function = 'r:=x^2+y^2;x+if(r<1,10,0)'
      function = 'cos(2*x)'
    [../]
  [../]
[]

[AuxVariables]
  [./u_grad_x]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./u_grad_x_x]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./ugrad_x]
    type = VariableGradientComponent
    variable = u_grad_x
    gradient_variable = u
    component = x
    execute_on = 'initial timestep_end'
  [../]
  [./ugrad_x_x]
    type = VariableGradientComponent
    variable = u_grad_x_x
    gradient_variable = u_grad_x
    component = x
    execute_on = 'initial timestep_end'
  [../]
[]

[Kernels]
  #NOTE: if the diffusion problem is turned on, then the solution oscillates
  [./diff]
    type = MatDiffusion
    D_name = 0.0
    variable = u
  [../]
  [./dt]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./dleft]
    type = DirichletBC
    value = -0.9899924966 #doesn't matter what the number is, just needs to be pinned somehow
    variable = u
    boundary = 'left'
  [../]
[]


[InterfaceKernels]
  [./xx_int_left]
    type = QuasiPeriodicTestKernel
    variable = u
    neighbor_var = u
    component = 0
    boundary = 'left'
    penalty = 1.0
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
  solve_type = 'NEWTON'  #"PJFNK, JFNK, NEWTON, FDP, LINEAR"
  dt = 0.1
  num_steps = 15
  dtmin = 0.1
[]

[Outputs]
  print_linear_residuals = true
  print_perf_log = true
  [./out]
    type = Exodus
    file_base = out_test_50_cx
    elemental_as_nodal = true
  [../]
[]
