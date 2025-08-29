[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 200
  ny = 3
  xmax = 1.0
  ymin = -0.05
  ymax = 0.05
[]

[GlobalParams]
  family = MONOMIAL
  order = CONSTANT
[]

[Variables]
  [h]
  []
  [hu]
  []
  [hv]
  []
[]

[Functions]
  [flat]
    type = ConstantFunction
    value = 0.0
  []
  [hL]
    type = ConstantFunction
    value = 1.0
  []
  [hR]
    type = ConstantFunction
    value = 0.5
  []
  [h_init]
    type = ParsedFunction
    value = "if(x<0.5, hL, hR)"
    # expression = 'tanh((x-0.5)*10)*0.25+0.75'
    vars = 'hL hR'
    vals = 'hL hR'
  []
[]

[UserObjects]
  [flux]
    type = SWENumericalFluxHLLC
    use_pvrs = true         # PVRS+q speeds (recommended)
    degeneracy_eps = 1e-8   # widen to 1e-6 if Newton still stalls
    blend_alpha = 0.0       # try 0.2–0.3 if you see grid-aligned shock wiggles
  []
  [wall]
    type = SWEWallBoundaryFlux
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[ICs]
  [h0]
    type = FunctionIC
    variable = h
    function = h_init
  []
  [hu0]
    type = ConstantIC
    variable = hu
    value = 0.0
  []
  [hv0]
    type = ConstantIC
    variable = hv
    value = 0.0
  []
[]

[Materials]
  [recon]
    type = SWERDGReconstruction
    h = h
    hu = hu
    hv = hv
  []
[]

[AuxVariables]
  [b_field]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [b_out]
    type = FunctionAux
    variable = b_field
    function = flat
  []
[]

[DGKernels]
  [flux_h]
    type = SWEFVFluxDGKernel
    variable = h
    h = h
    hu = hu
    hv = hv
    b_var = b_field
    numerical_flux = flux
  []
  [flux_hu]
    type = SWEFVFluxDGKernel
    variable = hu
    h = h
    hu = hu
    hv = hv
    b_var = b_field
    numerical_flux = flux
  []
  [flux_hv]
    type = SWEFVFluxDGKernel
    variable = hv
    h = h
    hu = hu
    hv = hv
    b_var = b_field
    numerical_flux = flux
  []
[]

[BCs]
  active = 'bch bchu bchv'
  [bch]
    type = SWEFluxBC
    variable = h
    boundary = 'left right top bottom'
    h = h
    hu = hu
    hv = hv
    boundary_flux = wall
  []
  [bchu]
    type = SWEFluxBC
    variable = hu
    boundary = 'left right top bottom'
    h = h
    hu = hu
    hv = hv
    boundary_flux = wall
  []
  [bchv]
    type = SWEFluxBC
    variable = hv
    boundary = 'left right top bottom'
    h = h
    hu = hu
    hv = hv
    boundary_flux = wall
  []
[]

[Kernels]
  [th]
    type = TimeDerivative
    variable = h
  []
  [thu]
    type = TimeDerivative
    variable = hu
  []
  [thv]
    type = TimeDerivative
    variable = hv
  []
[]

[VectorPostprocessors]
  [h]
    type = LineValueSampler
    end_point = '${fparse 1-1/400} 0 0'
    num_points = 200
    sort_by = x
    start_point = '${fparse 1/400} 0 0'
    variable = h
  []
[]

# [Preconditioning]
#   [fdp]
#     type = FDP
#     full = true
#   []
# []

[Executioner]
  type = Transient
  dt = 1e-3
  nl_abs_tol = 1e-12
  num_steps = 100
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = true
  print_linear_residuals = false
[]

