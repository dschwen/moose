# Verbose Output: Current Status and Quick Reference

## TL;DR

**Current State:** Verbose output is **partially working**. Debug messages are shown but many are hardcoded and don't respect the `verbose` flag.

**What Works:**
- Basic progress messages during kernel setup
- Variable splitting analysis output
- Kernel generation notifications

**What Needs Work:**
- var_diff() evaluation steps not shown
- Differentiation details not visible
- Expression simplification not traced
- Many DEBUG messages ignore the `verbose=true/false` setting

## Quick Usage

### Enable All Verbose Output

```ini
[AutomaticWeakForm]
  # Enable comprehensive debugging
  verbose = true
  debug_print_expressions = true
  debug_print_derivatives = true
  debug_print_simplification = true
  debug_print_weak_form = true
  debug_print_jacobian = true
[]
```

### What You'll See (Currently)

```
[DEBUG] AutomaticWeakFormAction::act() called with task: add_variable
  [DEBUG] Variable splitting is enabled, calling setupVariableSplitting()
  [DEBUG] Energy functional before split analysis: -(div(grad(T)))
  [DEBUG] Number of split variables generated: 1

[AutomaticWeakForm] Analyzing expression for variable splitting...

[AutomaticWeakForm] Adding Kernels
--- Processing variable: T ---
Generated weak form for T: -(div(grad(T)))
```

## Available Flags

| Flag | Purpose | Status |
|------|---------|--------|
| `verbose` | General step-by-step output | ⚠️ Partially checked |
| `debug_print_expressions` | Show parsed AST | ✓ Exists, ❌ Not fully used |
| `debug_print_derivatives` | Show differentiation steps | ✓ Exists, ❌ Not implemented |
| `debug_print_simplification` | Show simplification steps | ✓ Exists, ❌ Not implemented |
| `debug_print_weak_form` | Show final weak forms | ✓ Works |
| `debug_print_jacobian` | Show Jacobian terms | ✓ Exists, ❌ Not fully used |

## Example Output with verbose=true

**Running:**
```bash
./moose_test-opt -i var_diff_verbose_demo.i --color off
```

**Current Output:**
```
[DEBUG] setupVariableSplitting() called
  [DEBUG] Energy functional before split analysis: -(div(grad(T)))
  [DEBUG] Number of split variables generated: 1
    [DEBUG] Split variable: T_hess (order 2 of T)
      Definition: laplacian(T)

[AutomaticWeakForm] Analyzing expression for variable splitting...
  Found 1 variables requiring splitting

[AutomaticWeakForm] Adding Kernels
========================================

--- Processing variable: T ---
  [DEBUG] About to generate weak form for T
  Energy expression: -(div(grad(T)))

Generated weak form for T: -(div(grad(T)))

[AutomaticWeakForm] Generating kernel for variable: T
  - Kernel name: T_variational
  - Kernel type: VariationalKernelBase
```

## What's Missing (High Priority)

### 1. var_diff() Evaluation Details

**Currently:** Silent - no output showing how var_diff() works
**Needed:**
```
[AutomaticWeakForm] Evaluating var_diff() operator
  Input: var_diff(0.5*k*dot(grad(T), grad(T)), T)
  Step 1: Parse energy density → 0.5*k*dot(grad(T), grad(T))
  Step 2: Compute ∂f/∂(∇T) → k*grad(T)
  Step 3: Apply integration by parts → -div(k*grad(T))
  Result: -(div(grad(T)))
```

### 2. Differentiation Steps

**Currently:** No output
**Needed:**
```
[DifferentiationVisitor] Computing variational derivative
  Order 0: ∂f/∂T = 0
  Order 1: ∂f/∂(∇T) = k*grad(T)
  Max order: 1
```

### 3. Expression Simplification

**Currently:** No output
**Needed:**
```
[ExpressionSimplifier] Simplifying expression
  Input: (0.5 * (2.0 * grad(T)))
  Rule: Multiply nested constants → 1.0 * grad(T)
  Rule: Multiply by 1.0 → grad(T)
  Output: grad(T)
```

## Workarounds

### Get Maximum Information

1. **Enable all flags** (even if some don't work yet)
2. **Check console output** - most messages go to `_console`
3. **Look for [DEBUG] tags** - hardcoded debug output
4. **Check for "Generated weak form"** - shows results

### Example Input File

See `test/tests/variational/var_diff_verbose_demo.i` for a complete example.

## Implementation Status Details

### Files Involved

- `framework/src/actions/AutomaticWeakFormAction.C`
  - Lines 66-77: Flag definitions
  - Lines 156-161: Flag initialization
  - Lines 544+: Verbose output (partial)

- `framework/include/utils/automatic_weak_form/StringExpressionParser.h`
  - Line 734-771: `parseVarDiff()` - NO verbose output yet

- `framework/src/utils/automatic_weak_form/WeakFormGenerator.C`
  - Various: Some verbose checks, many missing

### Quick Fixes Needed

1. **Replace hardcoded Moose::out:**
```cpp
// Bad:
Moose::out << "[DEBUG] Message\n";

// Good:
if (_verbose)
  _console << "[AutomaticWeakForm] Message\n";
```

2. **Add var_diff logging:**
```cpp
// In parseVarDiff():
if (_verbose)
  _console << "[AutomaticWeakForm] Evaluating var_diff("
           << energy_density->toString() << ", " << var_name << ")\n";
```

## Recommendations

### For Users (Now)

**Use verbose flags** - they help, even if incomplete:
```ini
verbose = true
debug_print_weak_form = true
```

**Check output for:**
- Energy expression after var_diff() evaluation
- Split variable decisions
- Final weak forms

### For Developers (Future)

**Priority 1:** Add var_diff() step-by-step output
**Priority 2:** Respect verbose flags consistently
**Priority 3:** Add differentiation and simplification traces

See `VERBOSE_OUTPUT_STATUS.md` for complete implementation plan.

## Testing Verbose Output

```bash
# Create test file with verbose enabled
cat > test_verbose.i << 'EOF'
[AutomaticWeakForm]
  energy_expression = 'var_diff(0.5*k*dot(grad(T), grad(T)), T)'
  verbose = true
  debug_print_weak_form = true
  # ... other parameters ...
[]
EOF

# Run and capture output
./moose_test-opt -i test_verbose.i --color off 2>&1 | tee verbose_output.log

# Check what information was provided
grep -E "var_diff|Evaluating|Generated|Split" verbose_output.log
```

## Summary

**Status:** 🟡 Partially Implemented

**Works:**
- ✅ Flags are defined and can be set
- ✅ Some output respects `verbose` flag
- ✅ Split variable decisions are shown
- ✅ Kernel generation is logged

**Needs Work:**
- ❌ var_diff() evaluation is silent
- ❌ Many DEBUG messages are hardcoded
- ❌ Differentiation steps not shown
- ❌ Simplification not traced

**Bottom Line:** Users can get useful information, but it's incomplete. The infrastructure exists to make it great with focused improvements.
