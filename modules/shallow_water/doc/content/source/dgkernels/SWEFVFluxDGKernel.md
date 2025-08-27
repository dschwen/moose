# SWEFVFluxDGKernel

Assembles side fluxes for the 2D shallow-water equations in a cell-centered FV (rDG) fashion.

- Expects face-extrapolated `MaterialProperty` values `h`, `hu`, `hv` from a reconstruction
  material and passes an optional `b` (bathymetry) property to the numerical flux.
- Uses a `BoundaryFluxBase`-style numerical flux UserObject (e.g., `SWENumericalFluxHLL`).
- Builds full 3×3 Element/Neighbor Jacobian blocks for Newton solves.

!syntax description /DGKernels/SWEFVFluxDGKernel

Key parameters:

- `h`, `hu`, `hv`: coupled variables.
- `numerical_flux`: internal side flux userobject.

!syntax parameters /DGKernels/SWEFVFluxDGKernel

!syntax inputs /DGKernels/SWEFVFluxDGKernel

!syntax children /DGKernels/SWEFVFluxDGKernel
