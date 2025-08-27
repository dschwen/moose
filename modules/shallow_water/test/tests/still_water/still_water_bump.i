[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmax = 1.0
  ymax = 1.0
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
  [bump]
    type = ParsedFunction
    value = "0.1*exp(-100*((x-0.5)^2 + (y-0.5)^2))"
  []
  [eta]
    type = ConstantFunction
    value = 1.0
  []
  [h_init]
    type = ParsedFunction
    value = "max(eta-bump, 0)"
    vars = 'eta bump'
    vals = 'eta bump'
  []
[]

[UserObjects]
  [flux]
    type = SWENumericalFluxHLL
    gravity = 9.81
    dry_depth = 1e-6
  []
  [wall]
    type = SWEWallBoundaryFlux
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
    bed = bump
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
  [sx]
    type = SWEBedSlopeSource
    variable = hu
    h = h
    direction = x
    bed = bump
  []
  [sy]
    type = SWEBedSlopeSource
    variable = hv
    h = h
    direction = y
    bed = bump
  []
[]

[Executioner]
  type = Transient
  dt = 1e-2
  num_steps = 150
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
