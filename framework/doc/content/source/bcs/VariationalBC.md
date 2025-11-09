# VariationalBC

!syntax description /BCs/VariationalBC

## Overview

`VariationalBC` implements **natural boundary conditions** that arise automatically from energy functionals through integration by parts. When deriving weak forms from variational principles, boundary integrals appear that represent physical fluxes, moments, or other boundary quantities.

This kernel is **automatically generated** by [AutomaticWeakFormAction](../actions/AutomaticWeakFormAction.md) when boundary terms are extracted from the energy functional. Users typically don't create these manually.

## Mathematical Foundation

### Integration by Parts and Boundary Terms

When computing the weak form of an energy functional:
\begin{equation}
F[u] = \int_\Omega f(u, \nabla u, \nabla^2 u, \ldots) \, dx
\end{equation}

We apply integration by parts to transfer derivatives from field variables to test functions. This produces **boundary integrals**:

\begin{equation}
\int_\Omega C^k : \nabla^k v \, dx = (-1)^k \int_\Omega (\nabla^k \cdot C^k) v \, dx + \sum_{i=0}^{k-1} (-1)^i \int_{\partial\Omega} (\mathbf{n} \cdot \nabla^i C^k) \nabla^{k-1-i} v \, ds
\end{equation}

where:
- $C^k = \frac{\partial f}{\partial(\nabla^k u)}$ is the coefficient at derivative order $k$
- $\mathbf{n}$ is the outward unit normal on $\partial\Omega$
- $v$ is the test function

### The Boundary Residual

`VariationalBC` evaluates the boundary term:
\begin{equation}
R_{\partial\Omega} = \int_{\partial\Omega} \text{sign} \cdot (C^k \cdot \mathbf{n}) \cdot v \, ds
\end{equation}

where:
- **sign** alternates with derivative order: $(-1)^k$ for order $k$
- The coefficient $C^k$ comes from the variational derivative
- The normal contraction $C^k \cdot \mathbf{n}$ gives the boundary flux

## Physical Interpretations

### Example 1: Heat Diffusion

**Energy functional:**
\begin{equation}
F[T] = \int_\Omega \frac{k}{2}|\nabla T|^2 \, dx
\end{equation}

**Variational derivative:** $C^1 = k\nabla T$

**Boundary term:**
\begin{equation}
-\int_{\partial\Omega} k(\nabla T \cdot \mathbf{n}) v \, ds
\end{equation}

**Physical meaning:** This is the **Neumann (flux) boundary condition**:
- $k(\nabla T \cdot \mathbf{n})$ is the heat flux across the boundary
- If zero, the boundary is insulated
- If specified, it prescribes the heat flux

### Example 2: Cahn-Hilliard Phase Field

**Energy functional:**
\begin{equation}
F[c] = \int_\Omega \left[ W(c) + \frac{\kappa}{2}|\nabla c|^2 \right] dx
\end{equation}

**Boundary term:**
\begin{equation}
-\int_{\partial\Omega} \kappa(\nabla c \cdot \mathbf{n}) v \, ds
\end{equation}

**Physical meaning:** The natural boundary condition is $\nabla c \cdot \mathbf{n} = 0$ (no-flux). This means the phase interface meets the boundary at a **right angle**, which is the energetically favorable configuration.

### Example 3: Thin Plate Bending (Fourth-Order)

**Energy functional:**
\begin{equation}
F[w] = \int_\Omega \frac{D}{2}|\nabla^2 w|^2 \, dx
\end{equation}

**Boundary terms:** (two terms from fourth-order derivative)
\begin{equation}
\int_{\partial\Omega} D(\nabla^2 w \cdot \mathbf{n}) v \, ds + \int_{\partial\Omega} D(\nabla w \cdot \mathbf{n})(\nabla v \cdot \mathbf{n}) \, ds
\end{equation}

**Physical meaning:**
- First term: bending **moment** $M_n = D\nabla^2 w$
- Second term: **shear force** (Kirchhoff boundary condition)

## Implementation Details

### Expression Evaluation

The boundary expression is evaluated using `ExpressionEvaluator` at each quadrature point on the boundary:

```cpp
Real VariationalBC::computeQpResidual()
{
  setupEvaluationContext();

  // Evaluate: sign * (coefficient · normal)
  Real boundary_flux = _evaluator->evaluate(_boundary_expr);

  // Multiply by test function
  return boundary_flux * _test[_i][_qp];
}
```

### Jacobian Computation

Two modes for Jacobian:

#### 1. Automatic Differentiation (default)

```cpp
Real VariationalBC::computeQpJacobian()
{
  Real flux = _evaluator->evaluate(_boundary_expr);
  Real dflux_du = _evaluator->evaluateDerivative(_boundary_expr, _var.name());

  return dflux_du * _phi[_j][_qp] * _test[_i][_qp];
}
```

#### 2. Provided Expression

If Jacobian expression is provided explicitly:
```cpp
return _evaluator->evaluate(_jacobian_expr) * _test[_i][_qp];
```

### Normal Vector Handling

The outward unit normal $\mathbf{n}$ is available at each quadrature point:
- Stored in `_normals[_qp]` (from `IntegratedBC`)
- Registered in the evaluator as a special "variable" named `"n"`
- Used in the boundary expression (e.g., `dot(k*grad(u), n)`)

## Usage

### Manual Usage (Rare)

Typically `VariationalBC` is generated automatically, but manual usage is possible:

```ini
[BCs]
  [natural_bc]
    type = VariationalBC
    variable = T
    boundary = 'left right'
    boundary_expression = '-k * dot(grad(T), n)'  # Neumann BC for diffusion
    use_automatic_differentiation = true
    parameters = 'k 1.0'
  []
[]
```

### Automatic Generation (Typical)

More commonly, use `AutomaticWeakFormAction`:

```ini
[AutomaticWeakForm]
  energy_expression = '0.5*k*dot(grad(T), grad(T))'
  variables = 'T'
  parameters = 'k 1.0'

  # Boundary conditions will be automatically generated
  # on boundaries where Dirichlet conditions are not specified
[]

[BCs]
  # Only specify Dirichlet conditions
  [fixed_left]
    type = DirichletBC
    variable = T
    boundary = 'left'
    value = 0
  []
  # Natural BC on 'right' boundary is automatic
[]
```

## Parameters

| Parameter | Type | Required | Default | Description |
|-----------|------|----------|---------|-------------|
| `boundary_expression` | string | Yes | - | Boundary term expression including coefficient and normal contraction |
| `jacobian_expression` | string | No | "" | Optional Jacobian expression |
| `off_diagonal_jacobian_expressions` | map | No | {} | Off-diagonal Jacobian for coupled variables |
| `coupled_variables` | vector | No | [] | List of coupled variable names |
| `use_automatic_differentiation` | bool | No | true | Use AD to compute Jacobian |
| `parameters` | map | No | {} | Parameters used in expressions |

### Expression Syntax

**boundary_expression** should be of the form:
```
sign * dot(coefficient, n)
```

Examples:
- Heat diffusion: `'-1.0 * k * dot(grad(T), n)'`
- Phase field: `'-kappa * dot(grad(c), n)'`
- With sign included: `'-1.0 * dot(k * grad(T), n)'`

The **normal vector** `n` is automatically available in expressions.

## Examples

### Example 1: Insulated Boundary (Neumann BC)

For a diffusion problem with insulated boundary:

```ini
[BCs]
  [insulated]
    type = VariationalBC
    variable = T
    boundary = 'right'
    boundary_expression = '-k * dot(grad(T), n)'  # Will be zero at equilibrium
  []
[]
```

The residual enforces:
\begin{equation}
k\frac{\partial T}{\partial n} = 0
\end{equation}

### Example 2: Prescribed Flux

To prescribe a specific flux $q_0$:

```ini
[Functions]
  [prescribed_flux]
    type = ConstantFunction
    value = 10.0  # Heat flux
  []
[]

[BCs]
  [flux_bc]
    type = FunctionNeumannBC  # Standard MOOSE BC
    variable = T
    boundary = 'right'
    function = prescribed_flux
  []
[]
```

**Note:** For prescribed fluxes, use standard MOOSE `NeumannBC` or `FunctionNeumannBC` instead of `VariationalBC`. Use `VariationalBC` when the natural BC should be **free** (homogeneous Neumann, $\frac{\partial u}{\partial n} = 0$).

### Example 3: Phase Field No-Flux

For Cahn-Hilliard with natural no-flux:

```ini
[BCs]
  [no_flux]
    type = VariationalBC
    variable = c
    boundary = 'all'
    boundary_expression = '-kappa * dot(grad(c), n)'
    parameters = 'kappa 0.01'
  []
[]
```

This enforces $\nabla c \cdot \mathbf{n} = 0$ (phase interface perpendicular to boundary).

## Relationship to Other Boundary Conditions

### vs NeumannBC

**NeumannBC** (standard MOOSE):
- Prescribes a specific flux value: $\frac{\partial u}{\partial n} = g$
- User specifies $g$ explicitly

**VariationalBC** (variational):
- Enforces the natural BC from the energy functional
- Typically homogeneous: $\frac{\partial u}{\partial n} = 0$
- Correct coefficient and sign computed automatically

### vs DirichletBC

**DirichletBC**:
- Essential boundary condition: $u = u_0$ on $\partial\Omega$
- Enforced strongly (not in weak form)

**VariationalBC**:
- Natural boundary condition: flux/moment condition
- Enforced weakly (in weak form integral)
- Complementary to Dirichlet BCs

**Usage pattern:**
- Apply `DirichletBC` where $u$ is prescribed
- Apply `VariationalBC` (or nothing) where flux is natural
- Do **not** apply both on the same boundary

## Advanced Features

### Coupled Multi-Physics

For coupled problems, off-diagonal Jacobian terms are computed automatically:

```ini
[BCs]
  [coupled_bc]
    type = VariationalBC
    variable = u
    boundary = 'right'
    boundary_expression = '-(k_u * dot(grad(u), n) + coupling * dot(grad(v), n))'
    coupled_variables = 'v'
    use_automatic_differentiation = true
  []
[]
```

The Jacobian includes:
- On-diagonal: $\frac{\partial R}{\partial u}$
- Off-diagonal: $\frac{\partial R}{\partial v}$

### Time-Dependent Boundaries

For moving boundaries or time-dependent coefficients:

```ini
[Functions]
  [time_varying_k]
    type = ParsedFunction
    expression = '1.0 + 0.5*sin(t)'
  []
[]

[BCs]
  [time_bc]
    type = VariationalBC
    variable = T
    boundary = 'right'
    boundary_expression = '-k(t) * dot(grad(T), n)'
    # k(t) evaluated from material property or function
  []
[]
```

## Debugging and Verification

### Check Boundary Flux

Use postprocessors to verify the boundary flux:

```ini
[Postprocessors]
  [boundary_flux]
    type = SideIntegralVariablePostprocessor
    variable = T
    boundary = 'right'
    # Computes ∫_∂Ω T ds (for verification)
  []

  [boundary_flux_gradient]
    type = SideDiffusiveFluxIntegral
    variable = T
    boundary = 'right'
    diffusivity = k
    # Computes ∫_∂Ω k(∇T·n) ds (actual flux)
  []
[]
```

### Verify Natural BC

For a steady-state problem with no sources and insulated boundaries:
\begin{equation}
\int_{\partial\Omega} k\nabla T \cdot \mathbf{n} \, ds = 0
\end{equation}

This should be automatically satisfied if `VariationalBC` is used correctly.

### Common Issues

1. **Wrong sign on flux**
   - Check: Is the sign in `boundary_expression` correct?
   - The sign alternates with derivative order: $(-1)^k$

2. **Residual not converging**
   - Check: Are Dirichlet and Neumann BCs applied to different boundaries?
   - Cannot specify both types on the same boundary

3. **Expression evaluation errors**
   - Check: Is the normal vector `n` used correctly in expressions?
   - Use `dot(grad(u), n)` for scalar gradients

## Implementation Notes

### Expression Variables Available

Inside `boundary_expression`, you have access to:
- Field variables: `u`, `v`, etc. (values at QP)
- Gradients: `grad(u)`, `grad(v)`, etc.
- Normal vector: `n` (outward unit normal)
- Parameters: `k`, `kappa`, etc. (constants)
- Position: `x`, `y`, `z` (coordinates)
- Time: `t` (current time)

### Evaluation Context

At each boundary quadrature point:
```cpp
void VariationalBC::setupEvaluationContext()
{
  _evaluator->setFieldValue(_var.number(), _u[_qp]);
  _evaluator->setFieldGradient(_var.number(), _grad_u[_qp]);
  _evaluator->setFieldGradient(normal_id, _normals[_qp]);  // Normal vector
  _evaluator->setPosition(_q_point[_qp]);
  _evaluator->setTime(_t);
}
```

## References

### Mathematical Theory

1. **Variational Principles**: I.M. Gelfand and S.V. Fomin, "Calculus of Variations"
2. **Natural Boundary Conditions**: T.J.R. Hughes, "The Finite Element Method"
3. **Weak Forms**: J.N. Reddy, "An Introduction to the Finite Element Method"

### MOOSE Documentation

- [IntegratedBC](https://mooseframework.inl.gov/source/bcs/IntegratedBC.html)
- [BoundaryCondition System](https://mooseframework.inl.gov/syntax/BCs/)
- [AutomaticWeakFormAction](../actions/AutomaticWeakFormAction.md)

### Related Classes

- [VariationalKernelBase](../kernels/VariationalKernelBase.md) - Volume integral kernels
- [ExpressionEvaluationKernel](../kernels/ExpressionEvaluationKernel.md) - Similar expression evaluation
- [Boundary Term Extraction](../utils/BoundaryTermExtraction.md) - Mathematical extraction

## Summary

`VariationalBC` provides:
- ✅ Automatic evaluation of natural boundary conditions from energy functionals
- ✅ Correct signs and coefficients from integration by parts
- ✅ Automatic or manual Jacobian computation
- ✅ Support for coupled multi-physics problems
- ✅ Integration with automatic weak form generation

**Key principle:** Natural boundary conditions arise automatically from the physics (energy functional). `VariationalBC` enforces them correctly without manual derivation.

## Implementation

!syntax parameters /BCs/VariationalBC

!syntax inputs /BCs/VariationalBC

!syntax children /BCs/VariationalBC
