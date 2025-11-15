# Comprehensive Design for Automatic Weak Form Derivation in MOOSE

This document presents a complete, end-to-end design for implementing an **Automatic Weak Form Derivation System** for MOOSE.
It combines symbolic manipulation (integration by parts, variational derivatives, and derivative order enforcement) with existing MOOSE automatic weak form infrastructure.

---

## 1. Goals and Scope

The goal is to allow users to specify **strong-form PDEs** or **energy functionals** and automatically generate residuals, Jacobians, and natural boundary conditions without manually implementing weak forms.

**Key requirements:**

- Input: strong form PDEs with derivatives (`grad`, `div`, `laplacian`, partials).
- Output: weak form integrals with allowed derivative orders (up to gradients or Hessians).
- Perform **integration by parts (IBP)** to shift derivatives from field variables to test functions.
- Apply **variable splitting** when IBP cannot reduce order sufficiently.
- Integrate seamlessly with existing MOOSE automatic weak form and AD systems.

---

## 2. Leverage Existing Infrastructure

The existing **Automatic Weak Form System** already supports:

- AST and symbolic simplification
- Variational derivative computation
- AD-based Jacobians
- Variable splitting for higher-order terms【14†source】
- Integration into kernel generation【15†source】

We extend it with:

- A **StrongFormAnalyzer**
- An **IBPLowerer**
- A **BoundaryTermBuilder**
- A **DerivativeBudget** system (FE-order aware)

---

## 3. Mathematical Foundation

### Integration by Parts (General Form)

For tensor \(T\) and scalar \(w\):

\[
\int_\Omega (T : \nabla^k w) v \,dx = - \int_\Omega (\nabla \cdot T) : \nabla^{k-1} w \, v \, dx - \int_\Omega T : \nabla^{k-1} w \otimes \nabla v \, dx + \int_{\partial\Omega} (T \llcorner n) : \nabla^{k-1} w \, v \, ds.
\]

Repeat until derivatives of \(w\) are within allowed order.

---

## 4. Strong-Form Input Example

```ini
[AutomaticWeakForm]
  [heat]
    form = strong
    equations = 'dt(u) - div(k*grad(u)) = f'
    variables = 'u'
    parameters = 'k 1.0'
    boundary_terms = auto
  []
[]
```

---

## 5. Architecture Overview

### New Components

```
automatic_weak_form/
  StrongFormAnalyzer.h
  IBPLowerer.h
  BoundaryTermBuilder.h
  DerivativeBudget.h
```

### Extended Components

```
automatic_weak_form/
  ExpressionParser.h
  WeakFormGenerator.h
  VariableSplitting.h
  ExpressionSimplifier.h
```

---

## 6. Core Algorithms

### Derivative Budget

Compute allowed derivative order per variable from FE type:

- `coupledValue` → 0th
- `coupledGradient` → 1st
- `coupledSecond` → 2nd (if supported)【16†source】

### IBP Lowering

Apply tensor IBP recursively until derivative order ≤ FE budget.

### Boundary Term Builder

Collects and organizes natural boundary terms (Neumann/Robin/bending) for separate boundary kernels.

---

## 7. Integration by Parts Algorithm

```cpp
LowerOneStep(Term I, Var u, int target_order):
  choose index j to lower
  auto boundary = Boundary( (T ⟂ n_j) : nabla^{k-1} u * v );
  auto vol1 = - Vol( (div_j T) : nabla^{k-1} u * v );
  auto vol2 = - Vol( T : nabla^{k-1} u ⊗ nabla_j v );
  return { vol1 + vol2, boundary };
```

Repeat until all terms satisfy derivative constraints.

---

## 8. Variable Splitting

If the FE family does not support required derivatives (e.g., ∇²u for LAGRANGE elements), automatically introduce auxiliary variables:

- q = ∇u  
- r = ∇q  
- ...

These are enforced weakly by constraints like `q - grad(u) = 0`【14†source】.

---

## 9. Boundary Terms

Two canonical forms:

1. \( \int_{\partial\Omega} g(u,\nabla u) v \, ds \)
2. \( \int_{\partial\Omega} h(u,\nabla u) \partial_n v \, ds \)

Generated automatically unless `boundary_terms = none`.

---

## 10. Implementation Plan

1. Extend parser for strong-form operators.
2. Implement `StrongFormAnalyzer`.
3. Implement recursive `IBPLowerer`.
4. Implement `BoundaryTermBuilder`.
5. Integrate derivative budget checks.
6. Extend `VariableSplitting` to trigger when IBP fails.
7. Add new `StrongFormKernelBase` for generated kernels.
8. Add unit and integration tests for equivalence with energy formulations.

---

## 11. Example Walkthroughs

### Poisson Equation

Strong form: `-div(k*grad(u)) = f`

→ Weak form:
\[
\int_\Omega k\nabla u \cdot \nabla v = \int_\Omega f v + \int_{\partial\Omega} k (\partial_n u) v
\]

### Biharmonic (budget = 2)

Strong form: \( \Delta^2 u = f \)

Lowered form uses ∇²u, ∇²v and emits bending boundary terms.

If only first derivatives are supported → apply splitting.

---

## 12. Testing and Validation

- Unit tests for IBP algebra
- Manufactured solutions for diffusion, biharmonic, CH
- Compare weak forms against variational derivation【15†source】

---

## 13. Future Enhancements

- Automatic detection of conservative forms (`div(flux)`)
- IBP cost minimization heuristics
- Symbolic boundary classification (Dirichlet vs Neumann)
- Parallel symbolic simplification
- CAS-based verification

---

## References

- MOOSE Automatic Weak Form System【12†source】
- Variable Splitting Implementation【14†source】
- MOOSE Integration Plan【15†source】
- Coupleable Interface【16†source】

---

*Prepared for MOOSE Automatic Weak Form Project, 2025.*
