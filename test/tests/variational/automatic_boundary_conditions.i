# Test automatic boundary condition generation from energy functionals
#
# This test demonstrates that natural boundary conditions are automatically
# generated from the energy functional. For the diffusion energy:
#   F[T] = ∫ ½k|∇T|² dx
#
# The natural BC is: -k(∇T·n) = 0 (insulated/no-flux)
#
# We prescribe Dirichlet BCs on left and right boundaries, leaving top/bottom
# boundaries to use the automatically generated natural BCs (no-flux).

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
  [T]
    order = FIRST
    family = LAGRANGE
  []
[]

[ICs]
  [T_ic]
    type = ConstantIC
    variable = T
    value = 0.5
  []
[]

[AutomaticWeakForm]
  energy_type = expression
  energy_expression = '0.5*k*dot(grad(T), grad(T))'
  variables = 'T'
  parameters = 'k 1.0'
  verbose = true  # Show what boundary conditions are generated

  # Apply natural BCs only to top and bottom (exclude left/right which have Dirichlet BCs)
  natural_bc_boundaries = 'top bottom'
[]

[BCs]
  # Prescribe Dirichlet BCs on left and right
  [left]
    type = DirichletBC
    variable = T
    boundary = 'left'
    value = 1.0
  []

  [right]
    type = DirichletBC
    variable = T
    boundary = 'right'
    value = 0.0
  []

  # Top and bottom boundaries automatically get natural BCs:
  # VariationalBC with expression: -k(∇T·n) = 0
  # This means no heat flux (insulated boundaries)
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-10
  nl_max_its = 20

  l_tol = 1e-6
  l_max_its = 30

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]

[Postprocessors]
  # Verify the natural BC: flux through top/bottom should be zero
  [top_flux]
    type = SideDiffusiveFluxIntegral
    variable = T
    boundary = 'top'
    diffusivity = 1.0  # k = 1.0
  []

  [bottom_flux]
    type = SideDiffusiveFluxIntegral
    variable = T
    boundary = 'bottom'
    diffusivity = 1.0
  []

  # Total flux should be zero for steady-state with insulated boundaries
  [total_flux]
    type = LinearCombinationPostprocessor
    pp_names = 'top_flux bottom_flux'
    pp_coefs = '1.0 1.0'
  []
[]
