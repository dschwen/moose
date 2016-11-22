
[Mesh]
  type = GeneratedMesh
  dim = 2

  nx = 54
  ny = 54

  ymax = 3
  ymin = -3

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

[MeshModifiers]
  [./cnode1]
    type = AddExtraNodeset
    coord = '-3.0 0.0'
    new_boundary = 100
  [../]
  [./cnode2]
    type = AddExtraNodeset
    coord = '3.0 0.0'
    new_boundary = 101
  [../]
[]

[UserObjects]
  [./quasiperiodicneighbors]
    type = QuasiPeriodicNeighbors
    execute_on = 'initial linear nonlinear'
    auto_direction = 'x'
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      #function = 'r:=x^2+y^2;x+if(r<1,10,0)'
      function = '(x*y+2)^2'
    [../]
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
[]

[AuxKernels]
  [./ugrad_x]
    type = VariableGradientComponent
    variable = u_grad_x
    gradient_variable = u
    component = x
    execute_on = 'initial timestep_end'
  [../]
  [./ugrad_y]
    type = VariableGradientComponent
    variable = u_grad_y
    gradient_variable = u
    component = y
    execute_on = 'initial timestep_end'
  [../]
[]

[Kernels]
  [./diff]
    type = MatDiffusion
    D_name = 0.1
    variable = u
  [../]
  [./dt]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  #[./centerpin1]
  #  type = PresetBC
  #  value = 0.0
  #  boundary = '100'
  #  variable = u
  #[../]
  #[./centerpin2]
  #  type = PresetBC
  #  value = 0.0
  #  boundary = '101'
  #  variable = u
  #[../]
  [./dleft]
    type = DirichletBC #Don't REALLY need this. :D
    value = 4
    variable = u
    boundary = 'left'
  [../]
[]


[InterfaceKernels]
  #[./xx_int_left]
  #  type = QuasiPeriodicTestKernel
  #  variable = u
  #  neighbor_var = u
  #  component = 0
  #  boundary = 'left'
  #[../]
  [./xy_int_left]
    type = QuasiPeriodicTestKernel
    variable = u
    neighbor_var = u
    component = 1
    boundary = 'left'
  [../]
[]

[Problem]
  kernel_coverage_check = false
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
  dt = 0.5
  num_steps = 500
  dtmin = 0.5
  nl_abs_tol = 1e-10
[]

[Outputs]
  print_linear_residuals = true
  print_perf_log = true
  [./out]
    type = Exodus
    file_base = out_test_2D_diff
    elemental_as_nodal = true
  [../]
[]
