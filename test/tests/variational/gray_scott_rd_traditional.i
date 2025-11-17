# Gray-Scott Reaction-Diffusion Model - Traditional MOOSE Approach
# This uses hand-written custom kernels instead of AutomaticWeakForm
#
# Model equations:
#   ∂u/∂t = Du·∇²u - u·v² + F·(1-u)
#   ∂v/∂t = Dv·∇²v + u·v² - (F+k)·v
#
# Compare with gray_scott_rd_test.i which uses AutomaticWeakForm

Du=2e-5
Dv=5e-6
F=0.04
k=0.06

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 128
  ny = 128
  xmin = 0
  xmax = 1.25
  ymin = 0
  ymax = 1.25
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[ICs]
  # u = 1.0 + small random noise to break symmetry
  [u]
    type = RandomIC
    variable = u
    min = 0.998
    max = 1.000
    seed = 2342
  []

  [v]
    type = FunctionIC
    variable = v
    function = 'r:=sqrt((x-${Mesh/xmax}/2)^2+(y-${Mesh/ymax}/2)^2); if(r<${Mesh/xmax}/16,1,0)'
  []
[]

# Traditional approach: Each term requires its own kernel
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

  # Diffusion terms (need custom kernel with coefficients)
  [u_diffusion]
    type = CoefDiffusion
    variable = u
    coef = ${Du}
  []
  [v_diffusion]
    type = CoefDiffusion
    variable = v
    coef = ${Dv}
  []

  # Reaction terms (custom kernels with hand-coded Jacobians)
  [u_reaction]
    type = GrayScottReactionU
    variable = u
    v = v
    F = ${F}
  []
  [v_reaction]
    type = GrayScottReactionV
    variable = v
    u = u
    F = ${F}
    k = ${k}
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

  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.02
    optimal_iterations = 8
    iteration_window = 2
    growth_factor = 1.5
    cutback_factor = 0.75
  []
  dtmin = 0.001
  dtmax = 10

  end_time = 1000
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  nl_max_its = 12

  # Scale diffusion coefficients directly in PETSc options
  # since we're using the standard Diffusion kernel
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
  print_linear_residuals = false

  [console]
    type = Console
    max_rows = 5
  []
[]

[Postprocessors]
  [u_min]
    type = ElementExtremeValue
    value_type = min
    variable = u
  []
  [u_max]
    type = ElementExtremeValue
    value_type = max
    variable = u
  []
  [v_min]
    type = ElementExtremeValue
    value_type = min
    variable = v
  []
  [v_max]
    type = ElementExtremeValue
    value_type = max
    variable = v
  []
[]

# Note: This approach requires:
# - 3 custom C++ kernel classes (~250 lines total):
#   * CoefDiffusion (for Du·∇²u and Dv·∇²v)
#   * GrayScottReactionU (for -u·v² + F·(1-u))
#   * GrayScottReactionV (for u·v² - (F+k)·v)
# - Manual Jacobian derivation and implementation
# - Recompilation when equations change
#
# Compare with AutomaticWeakForm approach (gray_scott_rd_test.i):
# - No custom C++ code (just 10 lines in input file)
# - Automatic Jacobian (symbolic differentiation)
# - Just change the input file to modify equations
