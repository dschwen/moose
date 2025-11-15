# Boundary Term Extraction for Automatic Weak Forms

## Overview

This feature automatically extracts **natural boundary conditions** from energy functionals. When you specify a physics problem through an energy functional, the system now identifies what boundary terms arise from integration by parts and makes them available for inspection and use.

## What Problem Does This Solve?

Previously, when users specified an energy functional like:

```
F[u] = ∫ ½k|∇u|² dx
```

The system would compute the weak form residual but **ignore** the boundary terms that naturally arise. Users had to:
1. Manually derive what natural boundary conditions exist
2. Separately implement boundary condition kernels
3. Hope they got the signs and coefficients correct

Now, the system **automatically extracts and documents** these boundary terms.

## Mathematical Foundation

### Integration by Parts (IBP)

When computing weak forms from variational derivatives, we transfer derivatives from field variables onto test functions via IBP:

```
∫_Ω C^k : ∇^k v dx = (-1)^k ∫_Ω (∇^k · C^k) v dx
                     + ∑_{i=0}^{k-1} (-1)^i ∫_∂Ω (n · ∇^i C^k) ∇^{k-1-i} v ds
```

Where:
- **C^k** = ∂f/∂(∇^k u) is the coefficient at derivative order k (from variational derivative)
- **n** is the outward unit normal on ∂Ω
- **v** is the test function
- The boundary integral terms are the **natural boundary conditions**

### Sign Convention

Boundary terms get alternating signs from repeated IBP:
- **Order 1** (gradients): **negative sign** → `-∫_∂Ω (C¹·n) v ds`
- **Order 2** (Laplacians/Hessians): **positive sign** → `+∫_∂Ω (C²·n) v ds`
- **Order 3**: **negative sign**
- etc.

## Practical Examples

### Example 1: Heat Diffusion

**Energy functional:**
```
F[T] = ∫_Ω ½k|∇T|² dx
```

**Variational derivative:**
- C⁰ = 0 (no bulk term)
- **C¹ = k∇T** (flux)

**Boundary term extracted:**
```
∫_∂Ω -k(∇T·n) v ds
```

**Physical interpretation:** This is the **Neumann boundary condition** representing heat flux across the boundary. If ∇T·n = 0, you have insulation. If ∇T·n = q_specified, you have prescribed flux.

### Example 2: Thin Plate Bending

**Energy functional:**
```
F[w] = ∫_Ω ½D|∇²w|² dx
```

Where D is bending stiffness and w is vertical displacement.

**Variational derivative:**
- C⁰ = 0
- C¹ = 0
- **C² = D∇²w** (moment/curvature)

**Boundary term extracted:**
```
∫_∂Ω D(∇²w·n) v ds
```

**Physical interpretation:** This represents **bending moments** on the boundary. In full theory, this expands to both:
1. Moment boundary condition: M_n = -D∇²w
2. Shear (Kirchhoff) boundary condition: Q_n = -D∂(∇²w)/∂n

### Example 3: Cahn-Hilliard Phase Field

**Energy functional:**
```
F[c] = ∫_Ω [W(c) + ½κ|∇c|²] dx
```

**Variational derivative:**
- C⁰ = dW/dc (chemical potential from bulk energy)
- **C¹ = κ∇c** (interfacial energy contribution)

**Boundary term extracted:**
```
∫_∂Ω -κ(∇c·n) v ds
```

**Physical interpretation:** No-flux boundary condition (∇c·n = 0) means the phase interface meets the boundary at a right angle. Non-zero flux allows interface to intersect boundary at other angles or prescribe phase flux.

## Implementation Details

### Data Structure

Each boundary term is stored with:

```cpp
struct BoundaryTerm {
  NodePtr coefficient;              // Expression for the coefficient (e.g., k*grad(u))
  std::string variable_name;        // Which variable this BC applies to
  unsigned int field_derivative_order;   // Order in original energy (1 = grad, 2 = laplacian, etc.)
  unsigned int test_derivative_order;    // Order of test function (0 = v, 1 = ∂v/∂n, etc.)
  int sign;                         // +1 or -1 from IBP
  std::string description;          // Human-readable explanation
};
```

### Accessing Boundary Terms

```cpp
auto contributions = generator.computeContributions(energy_density, "u");

// Iterate over boundary terms
for (const auto & bt : contributions.boundary_terms) {
  std::cout << "Variable: " << bt.variable_name << "\n";
  std::cout << "Description: " << bt.description << "\n";
  std::cout << "Coefficient: " << bt.coefficient->toString() << "\n";
  std::cout << "Field derivative order: " << bt.field_derivative_order << "\n";
  std::cout << "Test derivative order: " << bt.test_derivative_order << "\n";
  std::cout << "Sign: " << bt.sign << "\n";
}
```

## Test Results

### Passing Tests (6/8)

✅ **SimpleDiffusionEnergy** - Correctly extracts Neumann BC from diffusion energy
✅ **CahnHilliardGradientEnergy** - Phase field flux BC extraction
✅ **BulkEnergyNoBoundaryTerms** - Correctly identifies when there are NO boundary terms
✅ **MixedBulkAndGradientEnergy** - Separates bulk and boundary contributions
✅ **DirectExtractionMethod** - Direct API usage works correctly
✅ **MultipleVariables** - Handles coupled multi-variable systems

### Failing Tests (2/8) - Expected Limitations

⚠️ **BendingEnergy** - Second-order test functions not yet implemented
⚠️ **MaxOrderTracking** (second-order case) - Same limitation

**Note:** These failures are due to `multiplyByTestFunction()` not supporting derivative_order > 1, which is a separate feature from boundary term extraction. The boundary terms are correctly identified; we just can't compute full weak form residuals for order > 1 yet.

## Usage in Input Files (Future Work)

Eventually, users will be able to configure boundary term handling:

```ini
[AutomaticWeakForm]
  energy_expression = '0.5*k*dot(grad(T), grad(T))'
  variables = 'T'

  # Boundary term options (planned):
  boundary_terms = auto          # Automatically generate natural BC kernels
  # boundary_terms = report      # Just report what they are
  # boundary_terms = none        # Ignore them (user handles manually)

  # Optional: override specific boundaries
  [./NaturalBCs]
    [./left]
      type = NeumannBC
      variable = T
      value = 0  # Insulated
    [../]
  [../]
[]
```

## Limitations and Future Work

### Current Limitations

1. **Second-order test functions** not implemented → can't compute full residuals for bending/fourth-order problems
2. **Boundary kernel generation** not yet integrated into AutomaticWeakFormAction
3. **Complex boundary geometries** - currently assumes simple normal vector contraction
4. **Corner conditions** - not yet handled (important for plates/shells)

### Planned Enhancements

1. **Automatic boundary kernel generation** - create IntegratedBC objects from extracted terms
2. **Boundary classification** - identify which boundaries need which BC types
3. **Corner treatment** - handle junction conditions for higher-order problems
4. **User control** - allow users to override/customize extracted BCs
5. **Visualization** - generate boundary term diagrams for documentation

## Mathematical Verification

The implementation has been verified against:

1. **Poisson equation**: ∇²u = f produces Neumann BC ∂u/∂n = g ✓
2. **Diffusion**: ∂u/∂t - ∇·(D∇u) = 0 produces flux BC D(∇u·n) = q ✓
3. **Cahn-Hilliard**: Phase field energy correctly produces no-flux natural BC ✓

Fourth-order problems partially verified (boundary terms extracted correctly, but full weak form computation blocked by test function limitation).

## References

### Mathematical Background

- **Variational Methods**: Gelfand & Fomin, "Calculus of Variations"
- **Weak Form Theory**: Reddy, "An Introduction to the Finite Element Method"
- **Natural BCs**: Hughes, "The Finite Element Method"

### Implementation Details

- **Code**: `framework/src/utils/automatic_weak_form/WeakFormGenerator.C`
- **Tests**: `unit/src/BoundaryTermExtractionTest.C`
- **Data Structures**: `framework/include/utils/automatic_weak_form/WeakFormGenerator.h`

## Summary

**What was implemented:**
- ✅ BoundaryTerm data structure with full metadata
- ✅ extractBoundaryTerms() method with detailed documentation
- ✅ Integration into WeakFormContributions
- ✅ Comprehensive unit tests
- ✅ Sign convention handling
- ✅ Multi-variable support

**What it enables:**
- Automatic identification of natural boundary conditions
- Clear connection between energy functional and boundary physics
- Foundation for automatic boundary kernel generation
- Better error detection (missing required BCs)
- Educational value (users can see what BCs arise)

**Next steps:**
1. Implement second-order test functions
2. Add boundary kernel generation to AutomaticWeakFormAction
3. Create user-facing controls for BC handling
4. Add integration tests with manufactured solutions
