[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 200
  ny = 1
  xmax = 1.0
  ymax = 0.005
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
    value = 0.1
  []
  [h_init]
    type = ParsedFunction
    value = "if(x<0.5, hL, hR)"
    vars = 'hL hR'
    vals = 'hL hR'
  []
[]

[UserObjects]
  [flux]
    type = SWENumericalFluxHLL
    gravity = 9.81
    dry_depth = 1e-6
    execute_on = 'INITIAL TIMESTEP_END'
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

[Executioner]
  type = ActuallyExplicitEuler
  dt = 1e-4
  num_steps = 500
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]

