# Minimal placeholder input for shallow water still-water test
# Note: This is a stub for anchoring the design; not wired into build.

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
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

[UserObjects]
  [flux]
    type = SWENumericalFluxHLL
    gravity = 9.81
    dry_depth = 1e-6
  []
[]

[ICs]
  [h0]
    type = ConstantIC
    variable = h
    value = 1.0
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

[Functions]
  [flat]
    type = ConstantFunction
    value = 0.0
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
  # Hydrostatic correction to preserve eta = const
  [corr_hu]
    type = SWEHydrostaticCorrectionDGKernel
    variable = hu
    h = h
    hu = hu
    hv = hv
  []
  [corr_hv]
    type = SWEHydrostaticCorrectionDGKernel
    variable = hv
    h = h
    hu = hu
    hv = hv
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
  # No explicit bed-slope source with hydrostatic reconstruction; it is
  # handled implicitly by the flux for well-balanced lake-at-rest.
[]

[Executioner]
  type = Transient
  dt = 1e-2
  num_steps = 1
[]

[Outputs]
  exodus = true
[]
