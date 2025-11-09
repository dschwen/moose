# LaplacianConstraintKernel

!syntax description /Kernels/LaplacianConstraintKernel

## Overview

`LaplacianConstraintKernel` enforces the constraint $q = \nabla^2 u$ (Laplacian of $u$) in a weak sense. This kernel is automatically generated when using [automatic weak form derivation](AutomaticWeakFormAction.md) for energy functionals containing fourth-order derivatives.

## Mathematical Foundation

### The Constraint

When dealing with fourth-order PDEs, finite elements of order $p$ can only represent derivatives up to order $p$. For standard first-order Lagrange elements, we can only compute first derivatives ($\nabla u$) directly. To handle higher-order terms, we introduce **auxiliary variables** to split the problem.

For a Laplacian constraint:
\begin{equation}
q = \nabla^2 u
\end{equation}

### Weak Form Derivation

The strong form constraint must be enforced weakly for finite element implementation. The naive approach would be:
\begin{equation}
\int_\Omega (q - \nabla^2 u) v \, dx = 0
\end{equation}

However, this requires computing $\nabla^2 u$, which is not available for first-order elements. Instead, we apply **integration by parts** to reduce the derivative order:

\begin{equation}
\begin{aligned}
\int_\Omega (q - \nabla^2 u) v \, dx &= \int_\Omega q v \, dx - \int_\Omega \nabla^2 u \cdot v \, dx \\
&= \int_\Omega q v \, dx + \int_\Omega \nabla u \cdot \nabla v \, dx - \int_{\partial\Omega} (\nabla u \cdot \mathbf{n}) v \, ds
\end{aligned}
\end{equation}

where $\mathbf{n}$ is the outward unit normal on the boundary $\partial\Omega$.

For problems with appropriate boundary conditions (Dirichlet on $u$, or natural BCs that cancel the boundary term), the weak form becomes:

\begin{equation}
\boxed{
\int_\Omega q v \, dx + \int_\Omega \nabla u \cdot \nabla v \, dx = 0
}
\end{equation}

This is the form implemented by `LaplacianConstraintKernel`.

## Implementation Details

The residual contribution for test function $\psi_i$ is:
\begin{equation}
R_i = \int_\Omega \left( q \psi_i + \nabla u \cdot \nabla \psi_i \right) dx
\end{equation}

At quadrature point $q_p$:
\begin{equation}
R_i = q_{q_p} \psi_i(q_p) + \nabla u_{q_p} \cdot \nabla \psi_i(q_p)
\end{equation}

### Jacobian Entries

The Jacobian matrix entries are:

**On-diagonal** (derivative with respect to $q$):
\begin{equation}
\frac{\partial R_i}{\partial q_j} = \int_\Omega \phi_j \psi_i \, dx = \phi_j(q_p) \psi_i(q_p)
\end{equation}

**Off-diagonal** (derivative with respect to $u$):
\begin{equation}
\frac{\partial R_i}{\partial u_j} = \int_\Omega \nabla \phi_j \cdot \nabla \psi_i \, dx = \nabla \phi_j(q_p) \cdot \nabla \psi_i(q_p)
\end{equation}

## Example: Fourth-Order Cahn-Hilliard

Consider the Cahn-Hilliard energy with higher-order gradient terms:

\begin{equation}
F[c] = \int_\Omega \left[ W(c) + \frac{\kappa}{2}|\nabla c|^2 + \frac{\lambda}{2}|\nabla^2 c|^2 \right] dx
\end{equation}

The fourth-order term $|\nabla^2 c|^2$ cannot be directly represented with first-order elements. The automatic weak form system introduces:

```
c_hess = ∇²c  (Laplacian of c)
```

and adds a `LaplacianConstraintKernel` to enforce this constraint:

!listing test/tests/variational/fourth_order_splitting.i block=Kernels/c_hess_constraint

## Usage

`LaplacianConstraintKernel` is typically generated automatically by [AutomaticWeakFormAction](AutomaticWeakFormAction.md) when variable splitting is required. Manual usage:

!listing

```
[Kernels]
  [q_constraint]
    type = LaplacianConstraintKernel
    variable = q        # The auxiliary variable representing ∇²u
    u = u              # The original variable
  []
[]
```

### Required Variables

You must define both variables in the `[Variables]` block:

```
[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
  [q]
    order = FIRST
    family = LAGRANGE
  []
[]
```

## Verification

The constraint can be verified by checking that $q \approx \nabla^2 u$ in the solution. For a manufactured solution $u = \sin(\pi x)\sin(\pi y)$:

- Expected: $\nabla^2 u = -2\pi^2 \sin(\pi x)\sin(\pi y)$
- Should match: $q = -2\pi^2 \sin(\pi x)\sin(\pi y)$

## Relationship to Other Kernels

- [GradientConstraintKernel](/GradientConstraintKernel.md) - First-order version ($\mathbf{q} = \nabla u$)
- [ScalarGradientConstraint](/ScalarGradientConstraint.md) - Specialized for 1D gradients
- [VectorGradientConstraint](/VectorGradientConstraint.md) - Specialized for vector gradients
- [VariationalKernelBase](/VariationalKernelBase.md) - Base class for variational kernels

## Mathematical References

1. **Variable Splitting**: P.G. Ciarlet, "The Finite Element Method for Elliptic Problems" (1978)
2. **Mixed Methods**: D. Boffi, F. Brezzi, M. Fortin, "Mixed Finite Element Methods and Applications" (2013)
3. **Fourth-Order Problems**: S. Brenner, L.R. Scott, "The Mathematical Theory of Finite Element Methods" (2008)

## Implementation

!syntax parameters /Kernels/LaplacianConstraintKernel

!syntax inputs /Kernels/LaplacianConstraintKernel

!syntax children /Kernels/LaplacianConstraintKernel
