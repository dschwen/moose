# Poisson equation solved via strong-form + IBP lowering
#  - Strong form: -div(k * grad(u)) + f = 0
#  - Lowered weak form: ∫ k * grad(u) · grad(v) dx = ∫ f v dx

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[AutomaticWeakForm]
  # One scalar variable
  variables = 'u'

  # Constant coefficients
  parameters = 'k 1.0 f 1.0'

  # Strong form equation (steady): u = -div(k*grad(u)) + f
  # Action will IBP-lower RHS and assemble weak form as c0*test + c1·grad(test)
  strong_forms = 'u = -div(k*grad(u)) + f'

  # Disable AD for this test to use lowered weak form kernel path
  use_automatic_differentiation = false
[]

[BCs]
  # Homogeneous Dirichlet on all boundaries for well-posedness
  [left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 1
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  l_tol = 1e-8
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]

