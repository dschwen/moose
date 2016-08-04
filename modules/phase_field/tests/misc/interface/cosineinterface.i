[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 3.14159265358979
  nx = 50
[]

[AuxVariables]
  [./eta_res]
  [../]
[]

[Variables]
  [./eta]
    [./InitialCondition]
      type = FunctionIC
      function = 0.999-x/3.2 # (cos(x)+1)/2
  [../]
[]

[Kernels]
  [./eta]
    type = CosineInterface
    variable = eta
    direction = '1 0 0'
    save_in = eta_res
  [../]
[]

#[Preconditioning]
#  [./fdp]
#    type = FDP
#    full = true
#  [../]
#[]

[BCs]
  [./left]
    type = DirichletBC
    variable = eta
    boundary = left
    value = 1.0
  [../]
  [./right]
    type = DirichletBC
    variable = eta
    boundary = right
    value = 0.0
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  line_search = none
  nl_rel_tol = 0.99999

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  petsc_options = '-snes_converged_reason'
[]

[Outputs]
  exodus = true
  execute_on = 'linear'
[]
