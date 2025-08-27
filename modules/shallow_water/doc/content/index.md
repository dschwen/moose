## ShallowWater module

Finite-volume/DG implementation scaffolding for the 2D shallow-water equations (SWE).

Objects currently available:

- SWEFVFluxDGKernel: DG side flux assembly for [h, hu, hv].
- SWENumericalFluxHLL: HLL/Rusanov numerical flux (with approximate Jacobians).
- SWERDGReconstruction: Face-extrapolated values (stub pass-through).
- SWEBedSlopeSource: Bed slope source term for momentum.

