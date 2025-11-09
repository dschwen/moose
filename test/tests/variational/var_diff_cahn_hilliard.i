# Cahn-Hilliard equation using var_diff() operator
# Energy: F[c] = ∫ [W(c) + ½κ|∇c|²] dx
# where W(c) = (c²-1)² is the double-well potential
# Result: ∂c/∂t = -M∇·∇(δF/δc) = -M∇²(dW/dc - κ∇²c)
#
# This requires a split formulation:
#   μ = var_diff(W(c) + 0.5*kappa*dot(grad(c), grad(c)), c)
#   ∂c/∂t = M∇²μ

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 40
[]

[Variables]
  [c]
    [InitialCondition]
      type = FunctionIC
      function = '0.5 + 0.1*cos(2*pi*x)'
    []
  []
  [mu]
    # Chemical potential
  []
[]

[Kernels]
  # Time derivative
  [c_dot]
    type = TimeDerivative
    variable = c
  []

  # Conservative dynamics: ∂c/∂t = ∇·(M∇μ)
  [c_diff]
    type = MatDiffusion
    variable = c
    diffusivity = M
    v = mu  # Diffusion driven by chemical potential gradient
  []

  # Chemical potential from variational derivative
  [mu_residual]
    type = VariationalKernelBase
    variable = mu
    energy_type = expression
    # Using var_diff to compute: μ = δF/δc = dW/dc - κ∇²c
    energy_expression = 'var_diff(W(c) + 0.5*kappa*dot(grad(c), grad(c)), c) - mu'
    parameters = 'kappa 0.01'
    coupled_vars = 'c'
    use_automatic_differentiation = true
  []
[]

[Materials]
  [constants]
    type = GenericConstantMaterial
    prop_names = 'M'
    prop_values = '1.0'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  dt = 0.01
  end_time = 1.0
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-8
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
