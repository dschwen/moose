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
    type = SWENumericalFluxHLLC
    gravity = 9.81
    dry_depth = 1e-6
  []
  [outlet]
    type = SWEFreeOutflowBoundaryFlux
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
  [bath]
    type = SWEBathymetry
    bed = flat
  []
[]

[DGKernels]
  [flux_h]
    type = SWEFVFluxDGKernel
    variable = h
    h = h
    hu = hu
    hv = hv
    numerical_flux = flux
  []
  [flux_hu]
    type = SWEFVFluxDGKernel
    variable = hu
    h = h
    hu = hu
    hv = hv
    numerical_flux = flux
  []
  [flux_hv]
    type = SWEFVFluxDGKernel
    variable = hv
    h = h
    hu = hu
    hv = hv
    numerical_flux = flux
  []
[]

[BCs]
  active = 'bch bchu bchv'
  [bch]
    type = SWEFluxBC
    variable = h
    boundary = 'left right'
    h = h
    hu = hu
    hv = hv
    boundary_flux = outlet
  []
  [bchu]
    type = SWEFluxBC
    variable = hu
    boundary = 'left right'
    h = h
    hu = hu
    hv = hv
    boundary_flux = outlet
  []
  [bchv]
    type = SWEFluxBC
    variable = hv
    boundary = 'left right'
    h = h
    hu = hu
    hv = hv
    boundary_flux = outlet
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
  type = Transient
  dt = 2e-4
  num_steps = 100
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
