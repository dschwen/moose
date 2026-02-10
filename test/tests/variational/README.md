# Automatic Weak Form Variational Tests

This directory contains tests for the automatic weak form generation system in MOOSE.

## Test Organization

### Quick Smoke Tests (< 5 seconds)
Fast tests to verify basic functionality:
- `test_quick_run` - Basic execution
- `test_verbose_output` - Parser and verbose mode
- `polynomial_ad` - Simple polynomial expressions

### Diffusion Tests
Basic parabolic PDEs:
- `diffusion_simple` - Standard diffusion without AD
- `diffusion_with_ad` - Diffusion with automatic differentiation
- `poisson_strong_ibp` - Integration by parts verification

### Allen-Cahn Tests (Non-conserved order parameter)
Phase field evolution without conservation:
- `allen_cahn_anisotropic` - Anisotropic interface energy
- `allen_cahn_fourfold` - Crystalline anisotropy

### Cahn-Hilliard Tests (Conserved order parameter)
Phase separation with conservation:
- `cahn_hilliard_no_ad` - Without automatic differentiation
- `cahn_hilliard_with_ad` - With automatic differentiation
- `cahn_hilliard_steady` - Steady-state solution
- `cahn_hilliard_transient` - Time evolution
- `cahn_hilliard_string_parser` - String expression parsing

### Variable Splitting Tests
Higher-order PDE handling:
- `cahn_hilliard_fourth_order_split` - 4th-order CH with automatic splitting
- `fourth_order_splitting` - General higher-order splitting
- `test_splitting_simple` - Basic gradient splitting

### Coupled Multi-Physics Tests
Multiple interacting PDEs:
- `coupled_cahn_hilliard_diffusion` - CH + diffusion coupling
- `coupled_cahn_hilliard_allen_cahn` - Conserved + non-conserved coupling

### Reaction-Diffusion Tests
Pattern formation systems:
- `gray_scott_basic` - Gray-Scott reaction-diffusion
- `gray_scott_improved_timestepping` - Optimized adaptive timestepping

### var_diff() Operator Tests
Energy functional variational derivatives:
- `var_diff_simple` - Basic variational derivative computation
- `double_well_gradient` - Double-well with gradients

### Boundary Condition Tests
Natural BC extraction:
- `automatic_boundary_conditions` - BC generation from weak forms

## Running Tests

### Run All Variational Tests
```bash
cd test
./run_tests -i tests/variational
```

### Run Specific Category
```bash
./run_tests -i tests/variational -p diffusion
./run_tests -i tests/variational -p cahn_hilliard
./run_tests -i tests/variational -p coupled
```

### Run Individual Test
```bash
./run_tests -i tests/variational -r test_name
```

### Run Quick Tests Only
```bash
./run_tests -i tests/variational --re="quick|simple"
```

## Test Coverage

The test suite validates:

1. **Parser Functionality**
   - Mathematical expression parsing
   - Strong form equation parsing
   - Parameter substitution
   - var_diff() operator

2. **Weak Form Generation**
   - Integration by parts (IBP)
   - Boundary term extraction
   - Coefficient decomposition (c0, c1)

3. **Variable Splitting**
   - Automatic detection of higher-order derivatives
   - Split variable creation
   - Constraint kernel generation

4. **Differentiation Modes**
   - Symbolic differentiation (default)
   - Automatic differentiation (MetaPhysicL)
   - Numerical differentiation (finite differences)

5. **Kernel Generation**
   - VariationalKernelBase kernels
   - Time derivative kernels
   - Constraint kernels for splits
   - Coupling between variables

6. **Physics Coverage**
   - Diffusion (parabolic)
   - Allen-Cahn (gradient flow)
   - Cahn-Hilliard (conserved dynamics)
   - Reaction-diffusion (Gray-Scott)
   - Coupled multi-physics

## Expected Test Times

| Category | Number of Tests | Approximate Time |
|----------|----------------|------------------|
| Quick/Smoke | 3 | < 10 seconds |
| Diffusion | 3 | ~30 seconds |
| Allen-Cahn | 2 | ~1 minute |
| Cahn-Hilliard | 5 | ~2 minutes |
| Variable Splitting | 3 | ~1 minute |
| Coupled | 2 | ~1 minute |
| Gray-Scott | 2 | ~2 minutes |
| var_diff/Misc | 4 | ~1 minute |
| **Total** | **28** | **~8-10 minutes** |

## Tolerances

- **Exodiff tests**: `rel_err = 1e-5` (field comparisons)
- **CSVDiff tests**: `rel_err = 1e-4` (postprocessor comparisons)

## Key Features Tested

### Strong Form Input
```ini
[AutomaticWeakForm]
  strong_forms = 'u_t = D*laplacian(u) + f'
  variables = 'u'
  parameters = 'D 1.0'
[]
```

### var_diff() for Energy Functionals
```ini
[AutomaticWeakForm]
  strong_forms = 'c = var_diff(W(c) + 0.5*kappa*dot(grad(c), grad(c)), c)'
  variables = 'c'
  parameters = 'kappa 0.01'
[]
```

### Automatic Variable Splitting
```ini
[AutomaticWeakForm]
  strong_forms = 'c = var_diff(F, c)'
  variables = 'c'
  enable_splitting = true    # Automatically splits higher-order derivatives
  max_fe_order = 1          # Forces splitting for 2nd+ derivatives
[]
```

### Coupled Systems
```ini
[AutomaticWeakForm]
  strong_forms = 'u_t = Du*laplacian(u) - u*v*v;
                  v_t = Dv*laplacian(v) + u*v*v'
  variables = 'u v'
  parameters = 'Du 2e-5 Dv 1e-5'
[]
```

## Adding New Tests

When adding new tests:

1. **Create input file**: `new_test.i` in this directory
2. **Add test spec**: Add entry to `tests` file with appropriate type
3. **Set requirement**: Clear description of what's being tested
4. **Choose test type**:
   - `Exodiff`: Compare exodus output files (fields)
   - `CSVDiff`: Compare CSV postprocessor output
   - `RunApp`: Just verify it runs and produces expected output
5. **Set tolerances**: Use `rel_err` appropriate for physics
6. **Add gold files**: Run test once to generate, then commit

## Continuous Integration

These tests are run automatically on:
- Pull requests
- Commits to main branch
- Nightly builds

Test failures indicate:
- Parser regressions
- Weak form generation errors
- Numerical changes in kernels
- Platform-specific issues

## Debugging Failed Tests

If a test fails:

1. **Run with verbose**:
   ```bash
   ./moose_test-opt -i tests/variational/failing_test.i AutomaticWeakForm/verbose=true
   ```

2. **Check weak form output**:
   Look for "Deriving weak form" and "c0/c1 coefficients" in output

3. **Compare exodus files**:
   ```bash
   exodiff -map -f failing_test_out.e gold/failing_test_out.e
   ```

4. **Check for numerical differences**:
   Small differences (< 1e-10) may be acceptable

5. **Verify split variables**:
   If splitting enabled, check that split variables were created

## Related Documentation

- [AutomaticWeakFormAction Documentation](../../../framework/doc/content/source/actions/AutomaticWeakFormAction.md)
- [VariationalKernelBase Documentation](../../../framework/doc/content/source/kernels/VariationalKernelBase.md)
- [CLAUDE.md](../../../CLAUDE.md) - Development guide
