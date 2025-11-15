# Verbose Output Status and Recommendations

## Current State

### Available Flags (AutomaticWeakFormAction.C:65-77)

```ini
[AutomaticWeakForm]
  verbose = false                      # General verbose output
  debug_print_expressions = false      # Show parsed expressions and AST
  debug_print_derivatives = false      # Show derivative calculations
  debug_print_simplification = false   # Show expression simplification
  debug_print_weak_form = false        # Show final weak form expressions
  debug_print_jacobian = false         # Show Jacobian expressions
[]
```

### Implementation Status

**✓ Implemented:**
- Flags are registered in `validParams()` (lines 66-77)
- Flags are stored in member variables (lines 156-161)
- `_verbose` flag is checked in several places (lines 544, 567, 578, 619, 679, 702, etc.)
- Basic output exists for:
  - Variable analysis
  - Split variable generation
  - Kernel addition
  - Boundary condition generation

**⚠️ Partially Implemented:**
- DEBUG output uses `Moose::out` **unconditionally** instead of checking `_verbose`
- `debug_print_expressions` flag exists but not consistently checked
- `debug_print_derivatives` flag exists but not used in DifferentiationVisitor
- `debug_print_simplification` flag exists but not used in ExpressionSimplifier
- `debug_print_weak_form` is checked but only shows final result, not steps

**❌ Not Implemented:**
- Step-by-step var_diff() evaluation
- Intermediate expression transformations
- Variable splitting decision logic
- Detailed kernel parameter setup

## What Users Need

### 1. var_diff() Step-by-Step Evaluation

**Desired Output:**
```
[AutomaticWeakForm] Evaluating var_diff() operator
  Input: var_diff(0.5*k*dot(grad(T), grad(T)), T)
  Energy density: 0.5*k*dot(grad(T), grad(T))
  Variable: T

  Step 1: Parse energy density
    Parsed: BinaryOp(*, Constant(0.5*k), FunctionCall(dot, [grad(T), grad(T)]))

  Step 2: Compute variational derivative δF/δT
    Computing ∂f/∂T (order 0): 0
    Computing ∂f/∂(∇T) (order 1): k*grad(T)
    Computing ∂f/∂(∇²T) (order 2): 0

  Step 3: Apply integration by parts
    Order 0 contribution: 0
    Order 1 contribution: -div(k*grad(T))

  Step 4: Construct Euler-Lagrange equation
    Result: -div(k*grad(T)) = -k*laplacian(T)

  ✓ var_diff() evaluation complete: -(div(grad(T)))
```

### 2. Variable Splitting Decision Logic

**Desired Output:**
```
[AutomaticWeakForm] Analyzing expression for variable splitting
  Energy: -(div(grad(T)))
  Max FE order available: 1

  Analyzing derivative orders:
    Variable T:
      - Contains grad(T): order 1 ✓ (within FE order)
      - Contains laplacian(T) = grad(grad(T)): order 2 ⚠️ (exceeds FE order)

  Decision: Splitting required for T
    Creating split variable: T_hess = laplacian(T)
    This reduces max order from 2 to 1

  Split variables created:
    1. T_hess (scalar)
       - Definition: laplacian(T)
       - Derivative order: 2
       - Parent variable: T
       - Constraint: T_hess - laplacian(T) = 0
```

### 3. Expression Transformation for Splitting

**Desired Output:**
```
[AutomaticWeakForm] Transforming energy for split variables
  Original: -(div(grad(T)))
  Split variables: T_hess

  Transformation rules:
    laplacian(T) → T_hess

  Transformed: -(div(grad(T)))  [no change - already first order after substitution]

  ✓ Transformation complete
```

### 4. Kernel Generation Details

**Desired Output:**
```
[AutomaticWeakForm] Generating kernel for variable: T
  Kernel name: T_variational
  Kernel type: VariationalKernelBase

  Parameters:
    - variable: T
    - energy_expression: -(div(grad(T)))
    - coupled_vars: [T_hess]
    - use_automatic_differentiation: true
    - enable_variable_splitting: true
    - fe_order: 1

  Coupled variables detected:
    - T_hess (from split variable system)

  ✓ Kernel added successfully
```

## Recommendations

### Short-Term (Easy Fixes)

1. **Replace hardcoded DEBUG with verbose flag checks:**
```cpp
// Current:
Moose::out << "[DEBUG] Message\n";

// Should be:
if (_verbose)
  _console << "[AutomaticWeakForm] Message\n";
```

2. **Add var_diff() evaluation output:**

In `StringExpressionParser::parseVarDiff()`:
```cpp
if (_verbose)
{
  _console << "\n[AutomaticWeakForm] Evaluating var_diff() operator\n";
  _console << "  Energy density: " << energy_density->toString() << "\n";
  _console << "  Variable: " << var_name << "\n";
}

// ... compute variational derivative ...

if (_verbose)
{
  _console << "  Result (Euler-Lagrange): " << euler_lagrange->toString() << "\n";
}
```

3. **Add splitting decision output:**

Already exists but needs improvement in `setupVariableSplitting()` (line 544-574).

### Medium-Term (Enhanced Output)

1. **DifferentiationVisitor verbose mode:**

Add a verbose flag to `DifferentiationVisitor` and output:
```cpp
if (verbose)
{
  _console << "  Computing ∂f/∂(∇^" << order << "u): ";
  _console << coefficient->toString() << "\n";
}
```

2. **ExpressionSimplifier step-by-step:**

Show before/after for each simplification rule applied.

3. **ExpressionTransformer detailed output:**

Show each substitution made during transformation.

### Long-Term (Complete Debugging Suite)

1. **Add `--debug-automatic-weak-form` command-line flag**

Automatically enables all debug flags for the automatic weak form system.

2. **Structured logging levels:**
```
Level 0 (default): Errors and warnings only
Level 1 (verbose): High-level steps
Level 2 (debug): Detailed steps and intermediate results
Level 3 (trace): Every operation and transformation
```

3. **Expression visualization:**

Optionally output AST trees in DOT format for graphical visualization.

4. **Performance profiling:**

Time each major step and report in summary.

## Current Workaround for Users

To get maximum debug information currently:

```ini
[AutomaticWeakForm]
  # Enable all available verbose flags
  verbose = true
  debug_print_expressions = true
  debug_print_derivatives = true
  debug_print_simplification = true
  debug_print_weak_form = true
  debug_print_jacobian = true
[]
```

**Note:** Some flags don't do anything yet, but enabling them prepares for future enhancements.

## Example: Current Output vs Desired Output

### Current Output (with verbose=true)

```
[AutomaticWeakForm] Analyzing expression for variable splitting...
  No variable splitting required (all derivatives within FE order 1)

[AutomaticWeakForm] Adding Kernels
========================================

--- Processing variable: T ---
  About to generate weak form for T
  Energy expression: -(div(grad(T)))
Generated weak form for T: -(div(grad(T)))

[AutomaticWeakForm] Generating kernel for variable: T
  - Kernel name: T_variational
  - Kernel type: VariationalKernelBase
```

### Desired Output

```
[AutomaticWeakForm] Evaluating var_diff() operator
  Input: var_diff(0.5*k*dot(grad(T), grad(T)), T)

  Step 1: Parse energy density
    Expression: 0.5*k*dot(grad(T), grad(T))
    AST: Multiply(Multiply(Constant(0.5), Parameter(k)), Dot(Grad(T), Grad(T)))

  Step 2: Compute variational derivative δF/δT
    ∂f/∂T (order 0): 0
    ∂f/∂(∇T) (order 1): 2*0.5*k*grad(T) = k*grad(T)

  Step 3: Construct Euler-Lagrange equation
    C⁰ = 0
    C¹ = k*grad(T)
    Euler-Lagrange: C⁰ - ∇·C¹ = -div(k*grad(T))
    Simplified: -k*laplacian(T)

  ✓ Result: -(div(grad(T)))

[AutomaticWeakForm] Analyzing expression for variable splitting
  Max FE order: 1
  Expression contains:
    ✓ grad(T): order 1 (within limit)

  Decision: No splitting required

[AutomaticWeakForm] Generating weak form for variable T
  Strong form: -(div(grad(T)))
  Weak form: ∫ dot(grad(T), grad(v)) dx

[AutomaticWeakForm] Adding kernel: T_variational
  Type: VariationalKernelBase
  Variable: T
  Energy: -(div(grad(T)))
  Automatic differentiation: ENABLED
  ✓ Kernel added successfully
```

## Implementation Priority

**High Priority (Do First):**
1. Replace hardcoded `Moose::out` with `_verbose` checks
2. Add var_diff() step output in `parseVarDiff()`
3. Improve splitting decision output (already partially there)

**Medium Priority:**
4. Add DifferentiationVisitor verbose mode
5. Add ExpressionTransformer verbose mode
6. Show kernel parameters clearly

**Low Priority (Nice to Have):**
7. Expression simplification steps
8. AST visualization
9. Performance profiling

## Testing

After implementing verbose output improvements, test with:

```bash
# Run with all verbose flags enabled
./moose_test-opt -i var_diff_verbose_demo.i --color off > verbose_output.txt 2>&1

# Check that output contains expected sections
grep "var_diff() operator" verbose_output.txt
grep "Analyzing expression for variable splitting" verbose_output.txt
grep "Generating kernel" verbose_output.txt
```

## Conclusion

The verbose output infrastructure is **partially implemented**. The flags exist and some output is generated, but:

1. Much of it uses hardcoded `Moose::out` instead of checking flags
2. Key steps (var_diff evaluation, differentiation, simplification) lack detailed output
3. Users would benefit from more structured, step-by-step information

**Recommendation:** Prioritize adding var_diff() evaluation output since that's the new feature users will most want to debug.
