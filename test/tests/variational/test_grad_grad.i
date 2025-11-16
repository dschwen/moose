# Test if grad(grad(c)) shape is handled correctly
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmin = 0
  xmax = 1
[]

[Variables]
  [c]
    order = FIRST
    family = LAGRANGE
  []
[]

[AutomaticWeakForm]
  # Strong form with second derivatives via var_diff
  strong_forms = 'c = var_diff(trace(grad(grad(c))), c)'
  variables = 'c'
  
  verbose = true
  debug_print_expressions = true
  debug_print_weak_form = true
  
  use_automatic_differentiation = false
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
