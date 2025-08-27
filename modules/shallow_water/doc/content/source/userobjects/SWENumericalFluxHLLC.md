# SWENumericalFluxHLLC

Implements an HLLC numerical flux for the 2D shallow-water equations using a
1D HLLC Riemann solver along the face normal with hydrostatic reconstruction.

- Supports optional bathymetry input (fourth entry `b`) for lake-at-rest preservation.
- Tangential velocity is advected unchanged across the contact wave.
- Provides approximate analytic Jacobians (based on physical flux + Rusanov contribution).

!syntax description /UserObjects/SWENumericalFluxHLLC

!syntax parameters /UserObjects/SWENumericalFluxHLLC

!syntax inputs /UserObjects/SWENumericalFluxHLLC

!syntax children /UserObjects/SWENumericalFluxHLLC

