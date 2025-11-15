# Variational Derivative Operator: `var_diff()`

## Overview

The `var_diff()` operator provides a unified way to specify PDEs in MOOSE by writing energy functionals directly. Instead of manually deriving Euler-Lagrange equations, you write the energy density and let the system compute the strong form PDE automatically.

## Syntax

```
var_diff(energy_density, variable)
```

**Arguments:**
- `energy_density`: Expression for the energy density f(u, ∇u, ∇²u, ...)
- `variable`: The field variable with respect to which the variational derivative is taken

**Returns:** The Euler-Lagrange equation (strong form PDE): C⁰ - ∇·C¹ + ∇²:C² - ...

## Mathematical Background

For a functional F[u] = ∫ f(x, u, ∇u, ∇²u, ...) dx, the variational derivative is:

```
δF/δu = ∂f/∂u - ∇·(∂f/∂(∇u)) + ∇²:(∂f/∂(∇²u)) - ...
```

The `var_diff()` operator computes this automatically using the chain rule and integration by parts.

## Examples

### Example 1: Heat Equation

**Energy functional:**
```
F[T] = ∫ ½k|∇T|² dx
```

**Input file:**
```ini
[AutomaticWeakForm]
  energy_type = expression
  energy_expression = 'var_diff(0.5*k*dot(grad(T), grad(T)), T)'
  parameters = 'k 1.0'
  variables = 'T'
[]
```

**Result:** Automatically generates the heat equation: ∂T/∂t = k∇²T

### Example 2: Cahn-Hilliard Equation

**Energy functional:**
```
F[c] = ∫ [W(c) + ½κ|∇c|²] dx
```

**Input file:**
```ini
[AutomaticWeakForm]
  energy_type = expression
  energy_expression = 'var_diff(W(c) + 0.5*kappa*dot(grad(c), grad(c)), c)'
  parameters = 'kappa 0.01'
  variables = 'c'
  add_time_derivative = true
[]
```

**Result:** Generates: ∂c/∂t = -M(dW/dc - κ∇²c)

### Example 3: Fourth-Order PDE

**Energy functional:**
```
F[u] = ∫ ½λ|∇²u|² dx
```

**Input file:**
```ini
[AutomaticWeakForm]
  energy_type = expression
  energy_expression = 'var_diff(0.5*lambda*laplacian(u)*laplacian(u), u)'
  parameters = 'lambda 0.001'
  variables = 'u'
  enable_splitting = true  # Automatically splits into lower-order system
  max_fe_order = 1
[]
```

**Result:**
- Generates fourth-order PDE: λ∇⁴u = 0
- Automatically introduces q = ∇²u as auxiliary variable
- Creates system: ∇²u - q = 0 and ∇²q = 0

### Example 4: Coupled System

**Energy functional:**
```
F[c,T] = ∫ [W(c) + ½κ|∇c|² + ½k|∇T|² + β·c·T] dx
```

**Input file:**
```ini
[AutomaticWeakForm]
  energy_type = expression
  # Coupled energy
  energy_expression = 'W(c) + 0.5*kappa*dot(grad(c), grad(c)) +
                      0.5*k_th*dot(grad(T), grad(T)) + beta*c*T'
  parameters = 'kappa 0.01 k_th 1.0 beta 0.1'
  variables = 'c T'
  add_time_derivative = true
[]
```

**Result:** Generates coupled PDEs:
- ∂c/∂t = -(dW/dc - κ∇²c + βT)
- ∂T/∂t = k_th∇²T - βc

## Comparison: Old vs New Approach

### Old Approach (Manual Derivation)

```ini
[AutomaticWeakForm]
  energy_type = expression
  energy_expression = '0.5*kappa*dot(grad(c), grad(c))'
  # User must manually derive: residual = -kappa*laplacian(c)
[]
```

**Issues:**
- User must manually derive Euler-Lagrange equation
- Error-prone for complex energies
- Hard to verify correctness

### New Approach (Automatic with var_diff)

```ini
[AutomaticWeakForm]
  energy_type = expression
  energy_expression = 'var_diff(0.5*kappa*dot(grad(c), grad(c)), c)'
[]
```

**Benefits:**
- Automatic derivation - no manual math required
- Guaranteed mathematical consistency
- Works for arbitrarily complex energies
- Handles higher-order PDEs automatically

## Advanced Features

### Using var_diff with Multiple Variables

For multi-physics problems, use `var_diff()` for each variable:

```ini
[AutomaticWeakForm]
  energy_type = expression
  energy_expressions = 'c = var_diff(total_energy, c)
                       T = var_diff(total_energy, T)'
  expressions = 'total_energy = W(c) + 0.5*kappa*dot(grad(c), grad(c)) +
                                0.5*k_th*dot(grad(T), grad(T)) + beta*c*T'
  parameters = 'kappa 0.01 k_th 1.0 beta 0.1'
  variables = 'c T'
[]
```

### Combining with Strong Form Terms

You can mix `var_diff()` with direct strong form terms:

```ini
[AutomaticWeakForm]
  energy_type = expression
  # Diffusion from energy + reaction term
  energy_expression = 'var_diff(0.5*D*dot(grad(u), grad(u)), u) + k*u*(1-u)'
  parameters = 'D 1.0 k 0.5'
  variables = 'u'
[]
```

## Supported Operations

The `var_diff()` operator works with all expression parser operations:

**Differential operators:**
- `grad(u)` - gradient
- `div(v)` - divergence
- `laplacian(u)` - Laplacian (∇²u)
- `curl(v)` - curl

**Vector operations:**
- `dot(a, b)` - dot product
- `cross(a, b)` - cross product
- `norm(v)` - vector magnitude

**Tensor operations:**
- `trace(T)` - tensor trace
- `sym(T)` - symmetric part
- `dev(T)` - deviatoric part

**Special functions:**
- `W(c)` - double-well potential: (c²-1)²
- `gamma(eta)` - surface energy function

## Implementation Notes

1. **Symbolic Computation**: `var_diff()` uses symbolic differentiation to compute exact derivatives

2. **Integration by Parts**: Automatically performed to convert high-order derivatives to weak form

3. **Variable Splitting**: When `enable_splitting=true`, higher-order derivatives are automatically split into systems of first-order PDEs

4. **Performance**: Evaluation happens once at problem setup - no runtime overhead

## Migration Guide

### From energy_expression

**Before:**
```ini
energy_expression = '0.5*kappa*dot(grad(c), grad(c))'
# System computes variational derivative internally
```

**After (more explicit):**
```ini
energy_expression = 'var_diff(0.5*kappa*dot(grad(c), grad(c)), c)'
# Same result, but var_diff is explicit
```

### From strong_forms

**Before:**
```ini
strong_forms = 'c_t = -kappa*laplacian(c)'  # Manually derived
```

**After:**
```ini
energy_expression = 'var_diff(0.5*kappa*dot(grad(c), grad(c)), c)'
# Automatically derives the same equation
```

## Troubleshooting

### Issue: "Second argument to var_diff() must be a variable name"

**Cause:** The second argument is not a simple variable name.

**Solution:** Ensure the second argument is exactly a variable name defined in `variables`:
```ini
variables = 'c'
energy_expression = 'var_diff(..., c)'  # Correct
energy_expression = 'var_diff(..., grad(c))'  # Wrong
```

### Issue: Results don't match manual derivation

**Cause:** Sign conventions in integration by parts.

**Solution:** Check that your manual derivation accounts for:
- Integration by parts generates alternating signs: + for order 0, - for order 1, + for order 2, etc.
- Boundary terms are handled separately

### Issue: Fourth-order problems fail to converge

**Cause:** Fourth-order PDEs require C¹ continuity.

**Solution:** Enable variable splitting:
```ini
enable_splitting = true
max_fe_order = 1  # Use C⁰ elements with splitting
```

## References

- [Calculus of Variations](https://en.wikipedia.org/wiki/Calculus_of_variations)
- [Euler-Lagrange Equation](https://en.wikipedia.org/wiki/Euler%E2%80%93Lagrange_equation)
- Phase-field modeling literature for energy functional examples

## See Also

- `AUTOMATIC_WEAK_FORM_DOCUMENTATION.md` - Complete system documentation
- `VARIABLE_SPLITTING_DOCUMENTATION.md` - Variable splitting for higher-order PDEs
- Test files in `tests/variational/` for working examples
