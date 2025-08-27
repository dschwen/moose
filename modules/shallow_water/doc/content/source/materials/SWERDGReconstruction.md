# SWERDGReconstruction

Provides face-extrapolated values for `[h, hu, hv]` using linear reconstruction
and optional multi-D slope limiting. If no limiter is provided, passes cell averages.

!syntax description /Materials/SWERDGReconstruction

Key parameters:

- `h`, `hu`, `hv`: coupled variables (cell-averaged).
- `slope_limiting`: optional slope limiting UserObject (e.g., `SlopeLimitingBarthJespersen`).

!syntax parameters /Materials/SWERDGReconstruction

!syntax inputs /Materials/SWERDGReconstruction

!syntax children /Materials/SWERDGReconstruction
