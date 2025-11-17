# Gray-Scott Reaction-Diffusion Model using Automatic Weak Form
#
# This is a simplified test version using direct energy expressions.
# The full strong form version is in gray_scott_rd.i (when fully implemented).
#
# Model equations:
#   ∂u/∂t = Du·∇²u - u·v² + F·(1-u)
#   ∂v/∂t = Dv·∇²v + u·v² - (F+k)·v

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 100
  xmin = 0
  xmax = 2.5
  ymin = 0
  ymax = 2.5
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[ICs]
  [u_ic]
    type = ConstantIC
    variable = u
    value = 1.0
  []
  [v_ic]
    type = FunctionIC
    variable = v
    function = 'r := sqrt((x-1.25)^2 + (y-1.25)^2); if(r < 0.1, 1.0, 0.0)'
  []
[]

[Kernels]
  # Time derivatives
  [u_time]
    type = TimeDerivative
    variable = u
  []
  [v_time]
    type = TimeDerivative
    variable = v
  []

  # Diffusion terms using var_diff
  [u_diffusion]
    type = VariationalKernelBase
    variable = u
    use_lowered_weak_form = true
    c0_expression = '0.0'
    c1_expression = 'Du*grad(u)'
    parameters = 'Du 2e-5'
    use_automatic_differentiation = false
  []

  [v_diffusion]
    type = VariationalKernelBase
    variable = v
    use_lowered_weak_form = true
    c0_expression = '0.0'
    c1_expression = 'Dv*grad(v)'
    parameters = 'Dv 1e-5'
    coupled_vars = 'u'
    use_automatic_differentiation = false
  []

  # Reaction terms
  [u_reaction]
    type = CoupledForce
    variable = u
    v = u_reaction_func
  []
  [u_reaction_func]
    type = ParsedAux
    variable = u_reaction_func
    expression = '-u*v*v + F*(1-u)'
    coupled_variables = 'u v'
    constant_names = 'F'
    constant_expressions = '0.0367'
    execute_on = 'LINEAR'
  []

  [v_reaction]
    type = CoupledForce
    variable = v
    v = v_reaction_func
  []
  [v_reaction_func]
    type = ParsedAux
    variable = v_reaction_func
    expression = 'u*v*v - (F+k)*v'
    coupled_variables = 'u v'
    constant_names = 'F k'
    constant_expressions = '0.0367 0.0649'
    execute_on = 'LINEAR'
  []
[]

[AuxVariables]
  [u_reaction_func]
  []
  [v_reaction_func]
  []
[]

[BCs]
  [Periodic]
    [all]
      variable = 'u v'
      auto_direction = 'x y'
    []
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 1.0
  end_time = 1000

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true
  interval = 10
[]
