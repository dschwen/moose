# Gray-Scott Reaction-Diffusion Patterns

This document describes various parameter sets for the Gray-Scott model that produce different interesting patterns using the Automatic Weak Form system.

## Using the Automatic Weak Form System

The Gray-Scott model showcases the automatic weak form generation from **strong form PDEs**. Instead of manually writing kernels, we simply specify the PDEs:

```ini
[AutomaticWeakForm]
  energy_type = expression

  # Specify the PDEs in mathematical notation
  strong_forms = 'u_t = Du*laplacian(u) - u*v*v + F*(1-u);
                  v_t = Dv*laplacian(v) + u*v*v - (F+k)*v'

  variables = 'u v'
  parameters = 'Du 2e-5 Dv 1e-5 F 0.0367 k 0.0649'
  use_automatic_differentiation = true
[]
```

The system automatically:
- Derives the weak form from the strong form PDEs
- Handles time derivatives (recognizes `u_t`, `v_t` notation)
- Applies integration by parts to Laplacian terms
- Generates the Jacobian using automatic differentiation
- Creates all necessary kernels

## Model Equations

```
∂u/∂t = Du·∇²u - u·v² + F·(1-u)
∂v/∂t = Dv·∇²v + u·v² - (F+k)·v
```

## Standard Diffusion Coefficients

For all patterns below, use:
- Du = 2e-5
- Dv = 1e-5 (ratio Dv/Du = 0.5)

## Pattern Types

### 1. Mitosis (Cell Division) - **Default**
**Parameters:** F = 0.0367, k = 0.0649

Produces patterns that resemble cell division. Spots split and multiply over time.

```ini
parameters = 'Du 2e-5 Dv 1e-5 F 0.0367 k 0.0649'
```

### 2. Coral Growth
**Parameters:** F = 0.0545, k = 0.062

Creates branching coral-like structures that grow and spread across the domain.

```ini
parameters = 'Du 2e-5 Dv 1e-5 F 0.0545 k 0.062'
```

### 3. Fingerprint / Maze Patterns
**Parameters:** F = 0.055, k = 0.062

Produces maze-like or fingerprint-like patterns with winding channels.

```ini
parameters = 'Du 2e-5 Dv 1e-5 F 0.055 k 0.062'
```

### 4. Spots
**Parameters:** F = 0.03, k = 0.062

Creates stable, evenly-spaced spots.

```ini
parameters = 'Du 2e-5 Dv 1e-5 F 0.03 k 0.062'
```

### 5. Stripes
**Parameters:** F = 0.035, k = 0.065

Produces stripe patterns.

```ini
parameters = 'Du 2e-5 Dv 1e-5 F 0.035 k 0.065'
```

### 6. Waves / Spirals
**Parameters:** F = 0.014, k = 0.054

Creates wave-like or spiral patterns that propagate through the domain.

```ini
parameters = 'Du 2e-5 Dv 1e-5 F 0.014 k 0.054'
```

### 7. Pulsating Spots (Solitons)
**Parameters:** F = 0.046, k = 0.0594

Spots that pulse and move around the domain.

```ini
parameters = 'Du 2e-5 Dv 1e-5 F 0.046 k 0.0594'
```

### 8. Worms
**Parameters:** F = 0.058, k = 0.065

Creates worm-like moving structures.

```ini
parameters = 'Du 2e-5 Dv 1e-5 F 0.058 k 0.065'
```

## Modifying Parameters

To try different patterns, simply change the `F` and `k` values in the `parameters` line:

```ini
[AutomaticWeakForm]
  energy_type = expression
  strong_forms = 'u_t = Du*laplacian(u) - u*v*v + F*(1-u);
                  v_t = Dv*laplacian(v) + u*v*v - (F+k)*v'
  variables = 'u v'
  parameters = 'Du 2e-5 Dv 1e-5 F 0.055 k 0.062'  # <-- Change F and k here
  use_automatic_differentiation = true
[]
```

## Enabling Verbose Output

To see how the automatic weak form system processes your equations:

```ini
[AutomaticWeakForm]
  verbose = true                    # Shows all processing steps
  debug_print_weak_form = true      # Shows the derived weak forms
  debug_print_expressions = true    # Shows parsed expressions
[]
```

This will display:
- How the strong form equations are parsed
- The integration-by-parts transformation for Laplacian terms
- The final weak form residuals
- Which kernels are generated

## Alternative Formulations

### Using Energy Functional with var_diff()

You can also express reaction-diffusion as an energy minimization problem:

```ini
[AutomaticWeakForm]
  energy_type = expression

  # Energy functional form using var_diff()
  energy_expression = 'var_diff(0.5*Du*dot(grad(u), grad(u)) + 0.5*Dv*dot(grad(v), grad(v)) + potential(u,v), u) +
                       var_diff(0.5*Du*dot(grad(u), grad(u)) + 0.5*Dv*dot(grad(v), grad(v)) + potential(u,v), v)'

  # Note: This is more complex for reaction terms and strong form is clearer
[]
```

The **strong form** approach is much more natural for reaction-diffusion systems!

## Exploration Tips

1. **Parameter Space:** The interesting region is roughly:
   - F: 0.01 to 0.1
   - k: 0.045 to 0.07

2. **Initial Conditions:** Different initial perturbations can lead to different final patterns. Try modifying the IC:

   ```ini
   [v_ic]
     type = FunctionIC
     variable = v
     # Single spot (default):
     function = 'r := sqrt((x-1.25)^2 + (y-1.25)^2); if(r < 0.1, 1.0, 0.0)'

     # Multiple spots:
     # function = 'r1:=sqrt((x-0.8)^2+(y-0.8)^2); r2:=sqrt((x-1.7)^2+(y-1.7)^2); if(r1<0.1 | r2<0.1, 1.0, 0.0)'

     # Random noise:
     # function = '0.01*rand()'
   []
   ```

3. **Mesh Resolution:**
   - 200×200 (default): Good balance
   - 300×300 or 400×400: Captures finer details
   - Coarser meshes miss important dynamics

4. **Boundary Conditions:**
   - **Periodic** (default): Patterns wrap around edges - most natural
   - **No-flux**: Change to Neumann BCs for reflection at boundaries

5. **Time Scale:** Most patterns take 5,000-15,000 time units to develop fully. The adaptive time stepper handles this efficiently.

## Performance Notes

The automatic weak form system with AD is very efficient for this problem:
- Jacobian is computed automatically and exactly
- No manual kernel coding required
- Easy to experiment with equation modifications
- Typical solve: ~3-6 nonlinear iterations per time step

## Advantages of Automatic Weak Form Approach

1. **Simplicity:** Write PDEs in mathematical notation, not C++ kernels
2. **Flexibility:** Change parameters or equations without recompiling
3. **Correctness:** Automatic differentiation ensures exact Jacobian
4. **Experimentation:** Try new terms or coupling easily

For example, adding a third species is trivial:
```ini
strong_forms = 'u_t = Du*laplacian(u) - u*v*v + F*(1-u);
                v_t = Dv*laplacian(v) + u*v*v - (F+k)*v + alpha*w;
                w_t = Dw*laplacian(w) - alpha*w + beta*v'
variables = 'u v w'
```

## Visualization

When visualizing results in Paraview:

1. **Load the exodus file** from the Outputs directory
2. **Apply filters:**
   - Contour: Shows pattern boundaries clearly
   - Warp by Scalar: Creates 3D view of concentration
   - Threshold: Isolates regions above/below a value

3. **Recommended settings:**
   - Variable: `v` (this forms the visible patterns)
   - Colormap: "Viridis" or "Plasma"
   - Animate over time to see pattern evolution

4. **What to look for:**
   - `u` stays near 1.0 almost everywhere (background)
   - `v` creates the visible structures (0 to ~0.5)
   - Pattern formation starts at the seed and spreads

## References

- Karl Sims Interactive Tutorial: https://www.karlsims.com/rd.html
- Pearson, J. E. (1993). "Complex Patterns in a Simple System". Science 261 (5118): 189–192.
- Gray, P.; Scott, S. K. (1985). "Sustained oscillations and other exotic patterns of behavior in isothermal reactions". J. Phys. Chem. 89 (1): 22–32.

## Running the Example

```bash
# From the test directory:
cd /path/to/moose-automatic-weak-form/test

# Run with default (mitosis) parameters:
./moose_test-opt -i tests/variational/gray_scott_rd.i

# Run with different parameters (e.g., coral growth):
./moose_test-opt -i tests/variational/gray_scott_rd.i AutomaticWeakForm/parameters='Du 2e-5 Dv 1e-5 F 0.0545 k 0.062'

# Run with verbose output:
./moose_test-opt -i tests/variational/gray_scott_rd.i AutomaticWeakForm/verbose=true
```
