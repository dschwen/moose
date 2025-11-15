# Floating-Point Tolerance Improvements

## Problem
The expression simplifier was using hardcoded tolerance value `1e-10` for floating-point comparisons, which:
- Is not portable across different floating-point precisions (float vs double)
- May be too loose or too tight depending on the context
- Doesn't adapt to the magnitude of values being compared

## Solution Implemented

### Machine Epsilon Based Tolerances
Replaced all hardcoded `1e-10` tolerances with robust floating-point comparison functions that use `std::numeric_limits<Real>::epsilon()`.

### Comparison Strategy
```cpp
bool isNumericallyEqual(Real a, Real b,
                        Real relTol = std::numeric_limits<Real>::epsilon() * 100,
                        Real absTol = std::numeric_limits<Real>::epsilon() * 100)
{
  // Check for exact equality first (handles infinities and identical values)
  if (a == b)
    return true;

  Real diff = std::abs(a - b);

  // For values near zero, use absolute tolerance
  if (std::abs(a) < absTol && std::abs(b) < absTol)
    return diff < absTol;

  // For other values, use relative tolerance
  Real largest = std::max(std::abs(a), std::abs(b));
  return diff <= largest * relTol;
}
```

### Key Features
1. **Adaptive Precision**: Automatically adjusts based on `Real` type (float or double)
2. **Relative Tolerance**: For values away from zero, uses relative comparison
3. **Absolute Tolerance**: For values near zero, uses absolute comparison
4. **Multiplier Factor**: Uses 100 * epsilon for practical robustness

### Updated Comparisons

| Operation | Old Code | New Code |
|-----------|----------|----------|
| Check if zero | `std::abs(value) < 1e-10` | `isNumericallyZero(value)` |
| Check if one | `std::abs(value - 1.0) < 1e-10` | `isNumericallyOne(value)` |
| General equality | `std::abs(a - b) < 1e-10` | `isNumericallyEqual(a, b)` |
| Non-zero check | `std::abs(divisor) > 1e-10` | `!isNumericallyZero(divisor)` |

## Benefits

### 1. Precision Portability
- Works correctly whether `Real` is typedef'd as `float` or `double`
- For double: epsilon ≈ 2.22e-16, tolerance ≈ 2.22e-14
- For float: epsilon ≈ 1.19e-7, tolerance ≈ 1.19e-5

### 2. Robust Arithmetic
The tolerance correctly handles:
- `0.5 * 2.0 = 1.0` (exact)
- `0.25 * 4.0 = 1.0` (exact)
- `0.125 * 8.0 = 1.0` (exact)
- `0.1 * 0.1 * 100.0 ≈ 1.0` (with rounding)

### 3. Prevents False Positives
Values that are clearly different from 1.0 are not incorrectly simplified:
- `1.000001 * x` remains as `(1.000001 * x)`
- `1.0 + 1000*epsilon` is not treated as 1.0

## Test Coverage

Created comprehensive `ToleranceTest` suite that validates:
- Near-one simplification
- Near-zero simplification
- Exact arithmetic (0.5 * 2.0 = 1.0)
- Non-near values remain distinct
- Division by small non-zero values
- Nested multiplication chains
- Accumulating rounding errors

All 717 tests pass including:
- 9 ToleranceTest tests
- 13 WeakFormDerivationTest tests
- 7 SimplifierLimitationsTest tests

## Mathematical Justification

The choice of 100 * machine epsilon provides:
- **Safety margin**: Accounts for accumulation of rounding errors
- **IEEE 754 compliance**: Works with standard floating-point representations
- **Practical robustness**: Handles typical numerical computations without false equivalences

For IEEE 754 double precision:
- Machine epsilon: 2^-52 ≈ 2.22e-16
- Our tolerance: 100 * 2.22e-16 ≈ 2.22e-14
- This allows ~2 decimal digits of accumulated error

## Impact
The improved tolerance handling ensures the simplifier:
- Correctly simplifies `(0.5 * (2.0 * grad(u)))` to `grad(u)`
- Doesn't incorrectly simplify expressions that are numerically distinct
- Adapts automatically to the floating-point precision in use
- Is more robust and maintainable