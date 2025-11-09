# Simple demonstration of var_diff() operator
# Solves heat equation using energy functional approach
# Energy: F[T] = ∫ ½k|∇T|² dx
# PDE: ∂T/∂t = k∇²T

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 20
[]

[Variables]
  [T]
  []
[]

[ICs]
  [T_ic]
    type = FunctionIC
    variable = T
    function = 'if(x<0.3, 1.0, if(x>0.7, 1.0, 0.0))'
  []
[]

[AutomaticWeakForm]
  energy_type = expression
  # Using var_diff to automatically derive heat equation from energy functional
  energy_expression = 'var_diff(0.5*k*dot(grad(T), grad(T)), T)'
  parameters = 'k 1.0'
  variables = 'T'
  use_automatic_differentiation = true
  verbose = true
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  dt = 0.01
  end_time = 0.1
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
