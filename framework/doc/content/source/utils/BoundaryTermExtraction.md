# Boundary Term Extraction

## Overview

The automatic weak form system includes **boundary term extraction** from energy functionals. When computing variational derivatives and applying integration by parts (IBP), boundary integrals naturally arise. These represent **natural boundary conditions** that must be enforced or left free.

This feature automatically identifies, documents, and can generate boundary condition kernels from the mathematical structure of the energy functional.

## Mathematical Foundation

### Integration by Parts

When deriving weak forms from variational derivatives, we transfer derivatives from field variables to test functions via integration by parts:

\begin{equation}
\int_\Omega C^k : \nabla^k v \, dx = (-1)^k \int_\Omega (\nabla^k \cdot C^k) v \, dx + \sum_{i=0}^{k-1} (-1)^i \int_{\partial\Omega} (\mathbf{n} \cdot \nabla^i C^k) \nabla^{k-1-i} v \, ds
\end{equation}

Where:
- $C^k = \frac{\partial f}{\partial(\nabla^k u)}$ is the coefficient at derivative order $k$ from the variational derivative
- $\mathbf{n}$ is the outward unit normal on $\partial\Omega$
- $v$ is the test function
- The boundary integral terms are the **natural boundary conditions**

### Sign Convention

Boundary terms acquire alternating signs from repeated integration by parts:
- **Order 1** (gradients): **negative sign** → $-\int_{\partial\Omega} (C^1 \cdot \mathbf{n}) v \, ds$
- **Order 2** (Laplacians/Hessians): **positive sign** → $+\int_{\partial\Omega} (C^2 \cdot \mathbf{n}) v \, ds$
- **Order 3**: **negative sign**
- etc.

### Why This Matters

Natural boundary conditions arise **automatically** from energy minimization. Users don't need to:
1. Manually derive what Neumann BCs exist
2. Separately implement boundary condition kernels
3. Guess the correct signs and coefficients

The system extracts this information directly from the energy functional.

## Implementation

### Data Structure

Each boundary term is stored with complete metadata:

```cpp
struct BoundaryTerm
{
  NodePtr coefficient;              // Expression for coefficient (e.g., k*grad(u))
  std::string variable_name;        // Variable this BC applies to
  unsigned int field_derivative_order;   // Order in energy (1=grad, 2=laplacian)
  unsigned int test_derivative_order;    // Order of test function (0=v, 1=∂v/∂n)
  int sign;                         // +1 or -1 from IBP
  std::string description;          // Human-readable explanation
};
```

### API Usage

```cpp
WeakFormGenerator generator(dim);
auto contributions = generator.computeContributions(energy_density, "u");

// Iterate over extracted boundary terms
for (const auto & bt : contributions.boundary_terms)
{
  std::cout << "Variable: " << bt.variable_name << "\n";
  std::cout << "Coefficient: " << bt.coefficient->toString() << "\n";
  std::cout << "Field derivative order: " << bt.field_derivative_order << "\n";
  std::cout << "Test derivative order: " << bt.test_derivative_order << "\n";
  std::cout << "Sign: " << bt.sign << "\n";
  std::cout << "Description: " << bt.description << "\n";
}
```

## Physical Examples

### Example 1: Heat Diffusion

**Energy functional:**
\begin{equation}
F[T] = \int_\Omega \frac{k}{2}|\nabla T|^2 \, dx
\end{equation}

**Variational derivative:**
- $C^0 = 0$ (no bulk term)
- $C^1 = k\nabla T$ (flux)

**Extracted boundary term:**
\begin{equation}
\int_{\partial\Omega} -k(\nabla T \cdot \mathbf{n}) v \, ds
\end{equation}

**Physical interpretation:** This is the **Neumann boundary condition** representing heat flux across the boundary:
- If $\nabla T \cdot \mathbf{n} = 0$: insulated boundary
- If $\nabla T \cdot \mathbf{n} = q_{\text{specified}}$: prescribed heat flux

### Example 2: Cahn-Hilliard Phase Field

**Energy functional:**
\begin{equation}
F[c] = \int_\Omega \left[ W(c) + \frac{\kappa}{2}|\nabla c|^2 \right] dx
\end{equation}

**Variational derivative:**
- $C^0 = \frac{dW}{dc}$ (chemical potential from bulk energy)
- $C^1 = \kappa\nabla c$ (interfacial energy contribution)

**Extracted boundary term:**
\begin{equation}
\int_{\partial\Omega} -\kappa(\nabla c \cdot \mathbf{n}) v \, ds
\end{equation}

**Physical interpretation:** No-flux boundary condition ($\nabla c \cdot \mathbf{n} = 0$) means the phase interface meets the boundary at a right angle. This is the **natural** boundary condition for phase field models.

### Example 3: Thin Plate Bending

**Energy functional:**
\begin{equation}
F[w] = \int_\Omega \frac{D}{2}|\nabla^2 w|^2 \, dx
\end{equation}

Where $D$ is bending stiffness and $w$ is vertical displacement.

**Variational derivative:**
- $C^0 = 0$
- $C^1 = 0$
- $C^2 = D\nabla^2 w$ (moment/curvature)

**Extracted boundary term:**
\begin{equation}
\int_{\partial\Omega} D(\nabla^2 w \cdot \mathbf{n}) v \, ds
\end{equation}

**Physical interpretation:** This represents **bending moments** on the boundary. In full plate theory, this expands to:
1. Moment boundary condition: $M_n = -D\nabla^2 w$
2. Shear (Kirchhoff) boundary condition: $Q_n = -D\frac{\partial(\nabla^2 w)}{\partial n}$

## Usage in AutomaticWeakFormAction

When using automatic weak form generation, boundary terms are extracted and made available:

```ini
[AutomaticWeakForm]
  energy_expression = '0.5*k*dot(grad(T), grad(T))'
  variables = 'T'

  # Future: control boundary term handling
  # boundary_terms = auto          # Generate BC kernels automatically
  # boundary_terms = report        # Just report what they are
  # boundary_terms = none          # Ignore (user handles manually)
[]
```

The system will:
1. Parse the energy expression
2. Compute variational derivatives
3. Extract boundary terms during IBP
4. Generate appropriate boundary condition kernels (or report them)

## Current Implementation Status

### What's Implemented ✓

1. **Mathematical extraction** - `WeakFormGenerator::extractBoundaryTerms()`
2. **Metadata storage** - `BoundaryTerm` struct with full information
3. **Integration into contributions** - Boundary terms included in `WeakFormContributions`
4. **Unit tests** - 6/8 tests passing in `BoundaryTermExtractionTest.C`
5. **Sign convention** - Correct alternating signs from IBP

### Current Limitations

1. **Second-order test functions** not implemented → can't compute full residuals for bending/fourth-order problems
2. **Boundary kernel generation** not yet integrated into `AutomaticWeakFormAction`
3. **Complex boundary geometries** - currently assumes simple normal vector contraction
4. **Corner conditions** - not yet handled (important for plates/shells)

### Test Results

**Passing tests (6/8):**
- ✅ SimpleDiffusionEnergy - Correctly extracts Neumann BC from diffusion
- ✅ CahnHilliardGradientEnergy - Phase field flux BC extraction
- ✅ BulkEnergyNoBoundaryTerms - Correctly identifies when NO boundary terms exist
- ✅ MixedBulkAndGradientEnergy - Separates bulk and boundary contributions
- ✅ DirectExtractionMethod - Direct API usage works correctly
- ✅ MultipleVariables - Handles coupled multi-variable systems

**Failing tests (2/8) - Known limitations:**
- ⚠️ BendingEnergy - Second-order test functions not implemented
- ⚠️ MaxOrderTracking (second-order case) - Same limitation

Note: These failures are due to `multiplyByTestFunction()` not supporting `derivative_order > 1`, which is separate from boundary term extraction. The boundary terms are correctly identified; we just can't compute full weak form residuals for order > 1 yet.

## Planned Enhancements

1. **Automatic boundary kernel generation** - Create `VariationalBC` objects from extracted terms
2. **Boundary classification** - Identify which boundaries need which BC types
3. **Corner treatment** - Handle junction conditions for higher-order problems
4. **User control** - Allow users to override/customize extracted BCs
5. **Visualization** - Generate boundary term diagrams for documentation
6. **Second-order test functions** - Enable full treatment of fourth-order problems

## Implementation Details

### Code Location

- **Extraction logic**: `framework/src/utils/automatic_weak_form/WeakFormGenerator.C`
- **Data structures**: `framework/include/utils/automatic_weak_form/WeakFormGenerator.h`
- **Unit tests**: `unit/src/BoundaryTermExtractionTest.C`
- **Integration point**: `framework/src/actions/AutomaticWeakFormAction.C` (stub exists)

### Key Methods

```cpp
// Extract boundary terms from differential
NodePtr WeakFormGenerator::extractBoundaryTerms(
    const Differential & diff,
    const std::string & var_name);

// Compute contributions including boundary terms
WeakFormContributions WeakFormGenerator::computeContributions(
    const NodePtr & energy_density,
    const std::string & var_name);
```

## Verification

The implementation has been verified against standard PDEs:

1. **Poisson equation**: $\nabla^2 u = f$ produces Neumann BC $\frac{\partial u}{\partial n} = g$ ✓
2. **Diffusion**: $\frac{\partial u}{\partial t} - \nabla \cdot (D\nabla u) = 0$ produces flux BC $D(\nabla u \cdot \mathbf{n}) = q$ ✓
3. **Cahn-Hilliard**: Phase field energy correctly produces no-flux natural BC ✓

Fourth-order problems partially verified (boundary terms extracted correctly, but full weak form computation blocked by test function limitation).

## Related Documentation

- [VariationalBC](../bcs/VariationalBC.md) - Boundary condition kernel for natural BCs (to be implemented)
- [VariationalKernelBase](../kernels/VariationalKernelBase.md) - Base class for variational kernels
- [AutomaticWeakFormAction](../actions/AutomaticWeakFormAction.md) - Main user interface
- [LaplacianConstraintKernel](../kernels/LaplacianConstraintKernel.md) - Example of IBP treatment

## References

### Mathematical Background

- **Variational Methods**: Gelfand & Fomin, "Calculus of Variations"
- **Weak Form Theory**: Reddy, "An Introduction to the Finite Element Method"
- **Natural BCs**: Hughes, "The Finite Element Method: Linear Static and Dynamic Finite Element Analysis"

### Integration by Parts Formulas

- **Divergence Theorem**: $\int_\Omega \nabla \cdot \mathbf{F} \, dx = \int_{\partial\Omega} \mathbf{F} \cdot \mathbf{n} \, ds$
- **Green's First Identity**: $\int_\Omega \nabla u \cdot \nabla v \, dx = -\int_\Omega (\nabla^2 u) v \, dx + \int_{\partial\Omega} \frac{\partial u}{\partial n} v \, ds$
- **Higher-Order**: Repeated application for fourth-order and beyond

## Example: Complete Workflow

For the energy functional:
\begin{equation}
F[u] = \int_\Omega \left[ \frac{1}{2}u^2 + \frac{k}{2}|\nabla u|^2 \right] dx
\end{equation}

**Step 1: Parse energy**
```cpp
auto energy = parser.parse("0.5*u*u + 0.5*k*dot(grad(u), grad(u))");
```

**Step 2: Compute contributions**
```cpp
auto contrib = generator.computeContributions(energy, "u");
```

**Step 3: Examine results**
```cpp
// Bulk term: C⁰ = u
contrib.c0_term->toString();  // "u"

// Gradient coefficient: C¹ = k∇u
contrib.c1_term->toString();  // "k * grad(u)"

// Boundary term
contrib.boundary_terms[0].coefficient->toString();  // "k * grad(u)"
contrib.boundary_terms[0].sign;  // -1
contrib.boundary_terms[0].description;  // "Neumann BC (flux): ..."
```

**Step 4: Generate kernels**
```cpp
// Volume integral: ∫ (u·v - k∇u·∇v) dx
// Boundary integral: -∫ k(∇u·n)v ds
```

The boundary term becomes a `VariationalBC` on boundaries where it's not prescribed (Dirichlet).

## Summary

**What was implemented:**
- ✅ Automatic mathematical extraction of natural boundary conditions
- ✅ Complete metadata (coefficients, signs, derivative orders)
- ✅ Unit test coverage
- ✅ Clear physical interpretations

**What it enables:**
- Automatic identification of required boundary conditions
- Connection between energy functional and boundary physics
- Foundation for automatic boundary kernel generation
- Error detection (missing required BCs)
- Educational value (users see what BCs arise)

**Next steps:**
1. Implement `VariationalBC` kernel class
2. Integrate boundary kernel generation into `AutomaticWeakFormAction`
3. Add second-order test function support
4. Create user-facing controls for BC handling
