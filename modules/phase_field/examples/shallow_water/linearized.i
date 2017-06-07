#
# Example problem demonstrating the implementation for the linearized shallow water equations
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 150
  ny = 150
  xmin = -100
  ymin = -100
  xmax = 100
  ymax = 100
[]

[Variables]
  # water surface elevation
  [./eta]
  [../]

  # velocity potential
  [./phi]
  [../]
[]

[Functions]
  [./bulge]
    type = ParsedFunction
    value = 'r:=x^2+y^2; exp(-r/100)*0.05'
  [../]
  [./bathymetry]
    type = ParsedFunction
    value = 'r:=(x-30)^2+(y-50)^2; 1-exp(-r/100)*0.8'
  [../]
[]

[Materials]
  # bathymetry
  [./h]
    type = GenericFunctionMaterial
    prop_names  = h
    prop_values = bathymetry
    outputs = exodus
  [../]
[]

[ICs]
  [./eta]
    type = FunctionIC
    variable = eta
    function = bulge
  [../]
  [./phi]
    type = FunctionIC
    variable = phi
    function = 0.0
  [../]
[]

[Kernels]
  [./eta_dot]
    type = CoupledTimeDerivative
    variable = phi
    v = eta
  [../]
  [./phi_diff]
    type = MatDiffusion
    variable = phi
    D_name = h
  [../]

  [./phi_dot]
    type = CoupledTimeDerivative
    variable = eta
    v = phi
  [../]
  [./eta_react]
    type = MatReaction
    variable = eta
    mob_name = 9.81
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
    [../]
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
  scheme = bdf2

  petsc_options_iname = '-pc_type -sub_pc_type'
  petsc_options_value = 'asm      lu          '

  l_max_its = 30
  l_tol = 1e-4
  nl_max_its = 20
  nl_rel_tol = 1e-9

  dt = 2.0
  end_time = 200.0
[]

[Outputs]
  exodus = true
  print_perf_log = true
[]
