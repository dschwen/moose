# Test double-well potential with AD
# Energy: F[c] = ∫ (c^2-1)^2 dx
# This should give δF/δc = 4c(c^2-1) = 4c^3 - 4c

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
  xmin = -1
  xmax = 1
[]

[Variables]
  [c]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.5
  []
[]

[AutomaticWeakForm]
    # Strong form via var_diff: δ/δc (c^2-1)^2 = 4c(c^2-1)
    strong_forms = 'c = var_diff((c*c - 1.0)*(c*c - 1.0), c)'
    variables = 'c'
    use_automatic_differentiation = false
[]

[BCs]
  [left]
    type = DirichletBC
    variable = c
    boundary = left
    value = -1
  []
  [right]
    type = DirichletBC
    variable = c
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  
  l_tol = 1e-10
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
