# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview: Automatic Weak Form Generation for MOOSE

This project integrates automatic weak form generation from **strong form PDEs** into the MOOSE framework. Users specify physics through strong form equations, with energy functionals expressed via the `var_diff()` variational derivative operator.

### Key Innovation

Instead of manually implementing kernels for PDEs, users can:
1. Specify strong form PDEs with arbitrary differential operators
2. Use `var_diff(energy, variable)` to compute variational derivatives from energy functionals
3. The system automatically derives weak forms via integration-by-parts (IBP)
4. Weak form kernels are generated with correct residuals and Jacobians
5. Higher-order PDEs are automatically split into systems of lower-order equations

---

## Unified Architecture (Current State)

### Single Input Method: `strong_forms`

**All physics** are specified via the `strong_forms` parameter. There are two ways to write equations:

#### Option 1: Direct Strong Form PDEs
```ini
[AutomaticWeakForm]
  strong_forms = 'c_t = D*laplacian(c) - k*c'
  variables = 'c'
  parameters = 'D 1.0 k 0.1'
[]
```

#### Option 2: Energy Functionals via var_diff()
```ini
[AutomaticWeakForm]
  strong_forms = 'c = var_diff(W(c) + 0.5*kappa*dot(grad(c), grad(c)), c)'
  variables = 'c'
  parameters = 'kappa 0.01'
[]
```

The `var_diff(energy_density, variable)` operator computes the Euler-Lagrange equation:
```
var_diff(F, u) = δF/δu = Σ(-1)^k ∇^k · (∂f/∂(∇^k u))
```

### Pipeline Architecture

```
User Input: strong_forms = '...'
     ↓
StringExpressionParser::parseStrongForms()
  [var_diff() operator computes δF/δu during parsing]
     ↓
StrongFormEquation objects (RHS expressions)
     ↓
StrongFormLowerer::lowerToWeak()
  [Integration by parts → c0 & c1 coefficients]
     ↓
VariationalKernelBase
  [Evaluates: R = c0*v + c1·∇v]
     ↓
Residual & Jacobian (symbolic or AD)
```

**Key Insight**: The `var_diff()` operator already exists in the parser and computes variational derivatives during parsing. Energy functionals naturally flow through the same strong form path.

---

## MOOSE Integration

### Core Components

1. **AutomaticWeakFormAction** (`framework/src/actions/AutomaticWeakFormAction.C`)
   - Main action that orchestrates automatic weak form generation
   - Parses strong form equations via `StringExpressionParser`
   - Derives weak forms via `StrongFormLowerer` (integration by parts)
   - Handles variable splitting for higher-order PDEs
   - Manages coupled multi-physics problems

2. **StrongFormLowerer** (`framework/src/utils/automatic_weak_form/StrongFormLowerer.C`)
   - Applies integration-by-parts to strong form RHS expressions
   - Produces c0 (multiplies test function) and c1 (multiplies grad test) coefficients
   - Extracts boundary terms for natural boundary conditions
   - Handles div, laplacian, and other differential operators

3. **VariationalKernelBase** (`framework/src/kernels/VariationalKernelBase.C`)
   - Base kernel class that implements weak forms
   - Evaluates residual: R = c0*v + c1·∇v
   - Computes Jacobians using symbolic differentiation or AD
   - Handles coupled variables automatically

4. **Constraint Kernels** (for variable splitting)
   - `ScalarGradientConstraint`: Enforces q = ∂u/∂x (component-wise)
   - `VectorGradientConstraint`: Enforces q = ∇u (vector variable)
   - `GradientConstraintKernel`: Legacy gradient constraints
   - `VariationalTimeDerivative`: Time derivatives for transient problems

5. **Expression Transformation** (`framework/src/utils/automatic_weak_form/`)
   - `StringExpressionParser`: Parses mathematical expressions, handles var_diff()
   - `ExpressionTransformer`: Transforms expressions for variable splitting
   - `ExpressionSimplifier`: Simplifies symbolic expressions
   - `WeakFormGenerator`: Computes variational derivatives and Euler-Lagrange equations
   - `VariableSplittingAnalyzer`: Detects high-order derivatives and generates split variables

---

## Mathematical Foundation

### Strong Form to Weak Form

For a strong form PDE:
```
du/dt = F(u, ∇u, ∇²u, ...)
```

The weak form is derived via integration by parts:
```
∫ (du/dt) v dx = ∫ F v dx
```

After IBP, the weak form becomes:
```
R = ∫ c0·v dx + ∫ c1·∇v dx + boundary terms
```

where:
- `c0`: Coefficient multiplying test function (bulk term)
- `c1`: Coefficient multiplying test function gradient (diffusion term)

### Energy Functionals to Strong Form

For an energy functional `F[u] = ∫ f(u, ∇u, ∇²u, ...) dx`:

The variational derivative is:
```
δF/δu = Σ_{k=0}^m (-1)^k ∇^k · (∂f/∂(∇^k u))
```

Gradient flow dynamics: `du/dt = -δF/δu`

**In code**: Use `var_diff(f, u)` which returns the strong form RHS.

---

## Build and Test Commands

```bash
# Build the MOOSE test app
cd test
make -j4

# Run specific test cases
./moose_test-opt -i tests/variational/cahn_hilliard_transient_ad.i
./moose_test-opt -i tests/variational/cahn_hilliard_fourth_order_split.i
./moose_test-opt -i tests/variational/gs_auto.i

# Run all variational tests
./run_tests -i tests/variational
```

### Known Build Issues

⚠️ **Pre-existing AD compilation error** in `VariationalKernelBase.C:952`:
- Issue: `std::pow` doesn't work with MetaPhysicL::DualNumber types
- Workaround: Use non-AD mode (`use_automatic_differentiation = false`)
- Fix needed: Replace `std::pow` with MetaPhysicL-compatible pow

---

## Current Capabilities

### 1. Expression Parsing

The `StringExpressionParser` supports:

**Basic Operations**:
- Arithmetic: `+`, `-`, `*`, `/`, `^`, `pow()`, `sqrt()`, `abs()`
- Functions: `exp()`, `log()`, `sin()`, `cos()`, `tan()`

**Differential Operators**:
- `grad(u)` or `gradient(u)` - Gradient (increases rank by 1)
- `div(V)` or `divergence(V)` - Divergence (decreases rank by 1)
- `laplacian(u)` or `laplace(u)` - Laplacian (∇²)
- `curl(V)` - Curl operator (3D only)
- `hessian(u)` - Hessian matrix (grad(grad(u)))

**Vector Operations**:
- `dot(v1, v2)` - Dot product
- `cross(v1, v2)` - Cross product (3D)
- `norm(v)` - Vector/tensor norm
- `normalize(v)` - Unit vector
- `outer(v1, v2)` - Outer product
- `vec(x, y)` or `vec(x, y, z)` - Vector assembly

**Tensor Operations**:
- `trace(T)` - Matrix trace
- `det(T)` - Determinant
- `inv(T)` - Matrix inverse
- `transpose(T)` or `trans(T)` - Transpose
- `sym(T)` or `symmetric(T)` - Symmetric part
- `skew(T)` - Skew-symmetric part
- `dev(T)` or `deviatoric(T)` - Deviatoric part
- `contract(T1, T2)` - Full contraction

**Special Functions**:
- `W(c)` or `doublewell(c)` - Double-well potential: (c² - 1)²
- `GinzburgLandau(psi)` - Ginzburg-Landau: (|ψ|² - 1)²
- `var_diff(energy, var)` - **Variational derivative operator** (Euler-Lagrange)

### 2. Variable Splitting

Automatically handles higher-order PDEs by introducing auxiliary variables:

**How it works**:
1. Detects when derivatives exceed FE capabilities (default: 1st order)
2. Introduces split variables: `q = ∇u`, `r = ∇²u`, etc.
3. Transforms strong form expressions to use split variables
4. Generates constraint kernels to enforce split definitions

**Example: 4th-order Cahn-Hilliard**
```
Original: f = W(c) + κ|∇c|² + λ|∇²c|²
Split: q = ∇²c, f = W(c) + κ|∇c|² + λ|q|²
Constraints: q = ∇²c (enforced by constraint kernel)
```

**Splitting Strategy**:
- Laplacian (∇²): Creates single scalar split variable
- Gradient (∇): Creates vector split variable (or component scalars)
- Smart detection: Only splits when necessary for FE order

### 3. Coupled Multi-Physics

Handles coupled systems automatically:
- Variables coupled based on expression dependencies
- Each kernel receives necessary coupled variables
- Supports arbitrary coupling patterns

**Example: Coupled Cahn-Hilliard + Heat**
```ini
strong_forms = 'c = var_diff(W(c) + 0.5*kappa*dot(grad(c), grad(c)) +
                              0.5*k_th*dot(grad(T), grad(T)) + beta*c*T, c);
                T_t = var_diff(W(c) + 0.5*kappa*dot(grad(c), grad(c)) +
                               0.5*k_th*dot(grad(T), grad(T)) + beta*c*T, T)'
```

Both variables use same energy but different variational derivatives.

### 4. Integration-by-Parts (IBP) Lowering

The `StrongFormLowerer` applies IBP to derive weak forms:

**Supported operators**:
- `div(V)` → `∫ div(V)·v dx = -∫ V·∇v dx + ∫ V·n·v ds`
- `laplacian(u)` → `∫ ∇²u·v dx = -∫ ∇u·∇v dx + ∫ (∇u·n)·v ds`
- Scalar multiplication
- Addition/subtraction

**Not yet supported** (see Future Work):
- curl operator
- Complete product rule for field-dependent terms
- Tensor divergence
- Nonlocal integral operators

### 5. Jacobian Computation

Multiple approaches available:
- **Symbolic differentiation** (exact, precomputed)
- **Automatic differentiation** (AD, runtime) - ⚠️ Currently has compilation issue
- **Finite differences** (fallback)

Set via `use_automatic_differentiation` parameter.

---

## Usage Examples

### Example 1: Diffusion Equation (Direct Strong Form)

```ini
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 50
[]

[Variables]
  [u][]
[]

[ICs]
  [u_ic]
    type = RandomIC
    variable = u
    min = -0.1
    max = 0.1
  []
[]

[AutomaticWeakForm]
  strong_forms = 'u_t = D*laplacian(u)'
  variables = 'u'
  parameters = 'D 1.0'
  use_automatic_differentiation = false  # Due to AD compilation issue
[]

[Executioner]
  type = Transient
  dt = 0.01
  end_time = 1.0
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
```

### Example 2: Cahn-Hilliard (Energy Functional)

```ini
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmin = -1
  xmax = 1
[]

[Variables]
  [c]
    [InitialCondition]
      type = FunctionIC
      function = '0.1*sin(3.14159*x)'
    []
  []
[]

[AutomaticWeakForm]
  # Energy: F = ∫ [W(c) + κ/2|∇c|²] dx
  # Gradient flow: ∂c/∂t = -δF/δc
  strong_forms = 'c_t = var_diff(W(c) + 0.5*kappa*dot(grad(c), grad(c)), c)'
  variables = 'c'
  parameters = 'kappa 0.01'
  use_automatic_differentiation = false
  verbose = true  # Show parsed expressions and weak forms
[]

[BCs]
  [left]
    type = NeumannBC
    variable = c
    boundary = left
    value = 0
  []
  [right]
    type = NeumannBC
    variable = c
    boundary = right
    value = 0
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 0.001
  end_time = 0.1

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
```

### Example 3: Fourth-Order with Variable Splitting

```ini
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 50
[]

[Variables]
  [c][]
[]

[ICs]
  [c_ic]
    type = RandomIC
    variable = c
    min = -0.1
    max = 0.1
  []
[]

[AutomaticWeakForm]
  # Energy with 4th-order term: F = ∫ [W(c) + κ|∇c|² + λ|∇²c|²] dx
  strong_forms = 'c = var_diff(0.25*(c^2 - 1)^2 + 0.5*kappa*dot(grad(c), grad(c)) +
                                0.01*dot(grad(grad(c)), grad(grad(c))), c)'
  variables = 'c'
  parameters = 'kappa 0.01'

  enable_splitting = true  # Automatically splits high-order derivatives
  max_fe_order = 1        # Use 1st-order elements (forces splitting)

  use_automatic_differentiation = false
  verbose = true  # Shows split variable creation
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
[]
```

### Example 4: Coupled Multi-Physics

```ini
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 50
[]

[Variables]
  [c][]   # Concentration
  [T][]   # Temperature
[]

[ICs]
  [c_ic]
    type = RandomIC
    variable = c
    min = 0.4
    max = 0.6
  []
  [T_ic]
    type = ConstantIC
    variable = T
    value = 1.0
  []
[]

[AutomaticWeakForm]
  # Coupled energy: F = ∫ [W(c) + κ|∇c|² + k|∇T|² + β·c·T] dx
  # Two equations from same energy:
  #   ∂c/∂t = -δF/δc
  #   ∂T/∂t = -δF/δT

  strong_forms = 'c_t = var_diff(W(c) + 0.5*kappa*dot(grad(c), grad(c)) +
                                  0.5*k_th*dot(grad(T), grad(T)) + beta*c*T, c);
                  T_t = var_diff(W(c) + 0.5*kappa*dot(grad(c), grad(c)) +
                                  0.5*k_th*dot(grad(T), grad(T)) + beta*c*T, T)'

  variables = 'c T'
  parameters = 'kappa 0.01 k_th 1.0 beta 0.1'
  use_automatic_differentiation = false
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 0.01
  end_time = 1.0
[]

[Outputs]
  exodus = true
[]
```

### Example 5: Mixed Energy and Explicit Terms

```ini
[AutomaticWeakForm]
  # Energy term via var_diff + explicit source term
  strong_forms = 'u_t = var_diff(0.5*k*dot(grad(u), grad(u)), u) + source*sin(x)'
  variables = 'u'
  parameters = 'k 1.0 source 0.1'
[]
```

### Example 6: Gray-Scott Reaction-Diffusion

```ini
[AutomaticWeakForm]
  # Classic Gray-Scott system
  strong_forms = 'u_t = Du*laplacian(u) - u*v*v + F*(1.0 - u);
                  v_t = Dv*laplacian(v) + u*v*v - (F + k)*v'

  variables = 'u v'
  parameters = 'Du 2e-5 Dv 1e-5 F 0.04 k 0.06'
  use_automatic_differentiation = false
[]
```

---

## Parameter Reference

### Required Parameters

- **`strong_forms`** (string): Strong form PDEs, semicolon-separated
  - Format: `'var = RHS; var2_t = RHS2'`
  - Use `var_t` or `dt(var)` for time derivatives
  - Use `var_diff(energy, var)` for variational derivatives

- **`variables`** (vector\<string>): List of primary variables
  - Space-separated: `'c T phi'`
  - Must match variables in strong_forms

### Optional Parameters

- **`parameters`** (map\<string, Real>): Parameter values
  - Space-separated key-value pairs: `'kappa 0.01 D 1.0'`

- **`coupled_variables`** (vector\<string>): Additional coupled variables

- **`expressions`** (string): Intermediate expressions
  - Semicolon-separated: `'strain = sym(grad(u)); stress = C*strain'`

- **`enable_splitting`** (bool, default: true): Enable variable splitting for high-order derivatives

- **`max_fe_order`** (int, default: 1): Maximum FE order (derivatives beyond this are split)

- **`use_automatic_differentiation`** (bool, default: true): Use AD for Jacobians
  - ⚠️ Set to `false` due to current AD compilation issue

- **`verbose`** (bool, default: false): Show detailed output
  - Displays parsed expressions, weak forms, split variables, kernels

### Debugging Parameters

- **`debug_print_expressions`** (bool): Print all parsed AST expressions
- **`debug_print_derivatives`** (bool): Print differentiation steps
- **`debug_print_simplification`** (bool): Show simplification steps
- **`debug_print_weak_form`** (bool): Print final weak form expressions
- **`debug_print_jacobian`** (bool): Print Jacobian terms

---

## Debugging Tips

### Enable Verbose Output

```ini
[AutomaticWeakForm]
  verbose = true
  debug_print_weak_form = true
  debug_print_jacobian = true
[]
```

**Verbose output shows**:
1. Parsed strong form equations
2. IBP lowering process (c0 and c1 coefficients)
3. Generated split variables (if any)
4. Created kernels with coupling information
5. Jacobian symbolic derivatives (if debug_print_jacobian = true)

### Common Issues

1. **"Variable not found" errors**
   - Ensure all variables in `strong_forms` are listed in `variables` parameter
   - Check spelling and case sensitivity

2. **Convergence problems**
   - Try smaller time steps
   - Use better initial conditions
   - Check constraint kernel scaling for split variables
   - Try different preconditioners: `petsc_options_iname = '-pc_type' petsc_options_value = 'lu'`

3. **Expression parsing errors**
   - Verify function names: `dot()` not `·`, `grad()` not `∇`
   - Check parentheses matching
   - Ensure parameters are defined before use
   - Use `vec(x,y,z)` for vector assembly

4. **Variable splitting issues**
   - Set `verbose = true` to see split variable creation
   - Verify boundary conditions are set for split variables
   - Check that `max_fe_order` is appropriate

5. **AD compilation error**
   - Workaround: Set `use_automatic_differentiation = false`
   - Uses symbolic differentiation instead (exact, may be slower to compile)

---

## File Structure

```
framework/
├── include/
│   ├── actions/
│   │   └── AutomaticWeakFormAction.h          # Main action header
│   ├── kernels/
│   │   ├── VariationalKernelBase.h            # Weak form kernel base class
│   │   ├── ScalarGradientConstraint.h         # Constraint: q_i = ∂u/∂x_i
│   │   ├── VectorGradientConstraint.h         # Constraint: q = ∇u (vector)
│   │   └── VariationalTimeDerivative.h        # Time derivative kernel
│   └── utils/automatic_weak_form/
│       ├── MooseAST.h                         # AST node types
│       ├── StringExpressionParser.h           # Expression parser
│       ├── StrongFormLowerer.h                # IBP lowering
│       ├── WeakFormGenerator.h                # Variational derivatives
│       ├── ExpressionTransformer.h            # Variable splitting transformer
│       ├── ExpressionSimplifier.h             # Symbolic simplification
│       └── VariableSplitting.h                # Splitting analyzer
│
├── src/
│   ├── actions/
│   │   └── AutomaticWeakFormAction.C          # ~1500 lines (after cleanup)
│   ├── kernels/
│   │   ├── VariationalKernelBase.C            # Weak form evaluation
│   │   ├── ScalarGradientConstraint.C         # Component-wise constraints
│   │   ├── VectorGradientConstraint.C         # Vector constraints
│   │   └── VariationalTimeDerivative.C        # Time derivatives
│   └── utils/automatic_weak_form/
│       ├── StringExpressionParser.C           # ~1200 lines, handles var_diff()
│       ├── StrongFormLowerer.C                # ~600 lines, IBP implementation
│       ├── WeakFormGenerator.C                # ~1000 lines, variational derivatives
│       ├── ExpressionTransformer.C            # Variable substitution
│       ├── ExpressionSimplifier.C             # Algebraic simplification
│       └── VariableSplitting.C                # Split variable generation
│
└── doc/content/source/
    ├── actions/AutomaticWeakFormAction.md
    └── kernels/
        ├── VariationalKernelBase.md
        ├── ScalarGradientConstraint.md
        ├── VectorGradientConstraint.md
        └── VariationalTimeDerivative.md

test/
├── tests/variational/
│   ├── cahn_hilliard_transient_ad.i           # Basic Cahn-Hilliard with var_diff
│   ├── cahn_hilliard_fourth_order_split.i     # 4th-order with splitting
│   ├── gs_auto.i                              # Gray-Scott system
│   └── coupled_*.i                            # Multi-physics examples
└── src/base/MooseTestApp.C                    # Registers all objects
```

---

## Testing

### Run Test Suite

```bash
cd test
make -j4
./run_tests -i tests/variational
```

### Key Test Files

- **`cahn_hilliard_transient_ad.i`**: Basic Cahn-Hilliard using var_diff()
- **`cahn_hilliard_fourth_order_split.i`**: Tests variable splitting with 4th-order term
- **`gs_auto.i`**: Gray-Scott reaction-diffusion (direct strong form)
- **`coupled_ch_diffusion_simple.i`**: Coupled multi-physics test

### Creating New Tests

1. Create `.i` file in `test/tests/variational/`
2. Use `strong_forms` parameter with var_diff() or direct PDEs
3. Set `use_automatic_differentiation = false` (until AD issue fixed)
4. Add gold file for regression testing
5. Run: `./run_tests --re <test_name>`

---

## Known Issues & Limitations

### 1. AD Compilation Error [BLOCKING]

**File**: `framework/src/kernels/VariationalKernelBase.C:952`

**Issue**: `std::pow(base, exp)` doesn't accept MetaPhysicL::DualNumber types

**Error message**:
```
no matching function for call to 'pow'
candidate function not viable: no known conversion from 'GenericReal<true>'
(aka 'MetaPhysicL::DualNumber<...>') to 'double'
```

**Workaround**: Set `use_automatic_differentiation = false` in input files

**Fix needed**: Replace `std::pow` with MetaPhysicL-compatible version or add proper type casting

**Impact**: AD mode unavailable; must use symbolic differentiation (slower compile, exact Jacobian)

### 2. Custom Function Expansion Incomplete

**File**: `framework/src/utils/automatic_weak_form/StringExpressionParser.C:25`

**Issue**: Custom function registry exists but expansion not fully implemented (TODO comment)

**Current**: `W()`, `gamma()` work (built-in)

**Missing**: User-defined functions via `registerFunction()` don't expand

**Impact**: Limited to predefined special functions

### 3. StrongFormLowerer Operator Coverage Incomplete

**File**: `framework/src/utils/automatic_weak_form/StrongFormLowerer.C`

**Supported**: div(), laplacian(), scalar multiplication, addition

**Missing**:
- curl() operator - Needs edge elements or splitting
- Complete product rule - Skips `V·grad(a)` when `a` depends on fields
- Tensor divergence - `div(tensor) → vector`
- Nonlocal integral operators

**Impact**: Some strong forms can't be lowered; produces errors or incorrect results

### 4. Boundary Conditions Not Auto-Generated

**File**: `framework/src/actions/AutomaticWeakFormAction.C:195`

**Issue**: Infrastructure exists but never invoked in action pipeline

**Current**: Users must specify BCs manually

**Missing**: Natural BCs from IBP boundary terms not automatically extracted

**Impact**: Extra work for users; natural BCs not guaranteed consistent

### 5. Parser Not Using MOOSE PEG Infrastructure

**Files**: `framework/src/utils/automatic_weak_form/StringExpressionParser.C`

**Current**: Custom recursive descent parser

**Issue**: MOOSE has PEG parser (used in CapabilityUtils) which is more maintainable

**Impact**: Harder to extend grammar, less robust error messages

### 6. Vector Splitting Creates Component Scalars

**File**: `framework/src/utils/automatic_weak_form/VariableSplittingAnalyzer.C`

**Issue**: Gradient splits create N scalar variables (q_x, q_y, q_z) instead of single vector variable

**Impact**: 3x memory overhead in 3D; more DOFs than necessary

---

## Roadmap & Future Work

### Immediate Goals (Current Sprint)

#### ✅ Completed
- [x] Fix split variable bug (variables now actually added to system)
- [x] Unify architecture (single strong_forms path, removed dual energy path)
- [x] Remove hard-coded energy types (~365 lines deleted)
- [x] Update variable splitting to use strong form equations

#### 🔄 In Progress
- [ ] Fix AD compilation error (VariationalKernelBase.C:952)
- [ ] Comprehensive testing of current implementation

---

### Short-Term Goals (Next 1-2 Weeks)

#### 1. Complete Custom Function Expansion [MEDIUM PRIORITY]
**Effort**: 2-3 hours

**Goal**: Enable user-defined energy functions

**Tasks**:
- Implement function registry with parameter substitution
- Complete TODO at StringExpressionParser.C:25
- Add expansion during parsing
- Test with custom functions

**Example usage after completion**:
```cpp
parser.registerFunction("myEnergy", "a*x^4 + b*x^2", {{"x", "u"}, {"a", "1.0"}, {"b", "-1.0"}});
// Then use: strong_forms = 'u = var_diff(myEnergy(u), u)'
```

#### 2. Extend StrongFormLowerer for More Operators [HIGH PRIORITY]
**Effort**: 1-2 days

**Goal**: Handle more differential operators correctly

**Tasks**:
- Add curl() operator IBP rules
- Complete product rule: properly handle `V·grad(a)` when `a` is field-dependent
- Add tensor divergence: `div(T)` where T is rank-2 tensor
- Add comprehensive test cases for each operator

**Operators to add**:
```cpp
// Curl: ∫ (curl u)·v dx = ∫ u·(curl v) dx + boundary term
case NodeType::Curl:
  return lowerCurlOperator(node);

// Product rule: ∫ f·grad(g) v dx = -∫ f·g·grad(v) dx + ∫ f·g·v·n ds
// Need to compute grad(g) symbolically using DifferentiationVisitor
```

#### 3. Enable Automatic Natural Boundary Conditions [MEDIUM PRIORITY]
**Effort**: 1 day

**Goal**: Extract and apply natural BCs automatically

**Tasks**:
- Call `addBoundaryConditions()` in `add_bc` action task
- Extract boundary terms from StrongFormLowerer during IBP
- Generate VariationalBC objects with correct coefficients
- Add parameter to enable/disable auto-BC generation

**Example**: For `∫ k∇u·∇v dx`, IBP produces boundary term `∫ k(∇u·n)v ds`.
If no Dirichlet BC specified, auto-generate natural BC: `k(∇u·n) = 0`.

---

### Medium-Term Goals (1-2 Months)

#### 4. Migrate to PEG Parser [LOW PRIORITY]
**Effort**: 1 week

**Goal**: Use MOOSE's PEG infrastructure for robustness

**Benefits**:
- Better error messages
- Easier to extend grammar
- More maintainable
- Standard MOOSE infrastructure

**Approach**:
- Define PEG grammar for mathematical expressions
- Keep StringExpressionParser as fallback during transition
- Gradually migrate functionality

#### 5. Optimize Vector Variable Splitting [MEDIUM PRIORITY]
**Effort**: 2-3 days

**Goal**: Use single vector variables instead of component scalars

**Current**: `q = grad(u)` creates `q_x`, `q_y`, `q_z` (3 scalars)

**After**: `q = grad(u)` creates single vector variable `q` (1 vector)

**Benefits**:
- Reduced DOFs (factor of 3 in 3D)
- Better memory layout
- Easier visualization

**Tasks**:
- Modify VariableSplittingAnalyzer to create vector variables
- Update ExpressionTransformer to handle vector references
- Update constraint kernels to work with vector variables
- Fallback to component scalars if vector constraints unavailable

#### 6. Implement Higher-Order Splitting Optimization [LOW PRIORITY]
**Effort**: 3-5 days

**Goal**: Smarter splitting for 3rd+ order derivatives

**Current**: Naive recursive splitting (r = ∇q, q = ∇u)

**Improvements**:
- Detect biharmonic: ∇²(∇²u) → create v = ∇²u (not nested grads)
- Operator fusion: div(grad(grad(u))) recognized as biharmonic
- Minimal splitting: Only split to FE capability, not beyond

**Example**:
```
4th-order: ∇²(∇²u)
Current: r = ∇q, q = ∇²u (2 split variables)
Optimal: v = ∇²u (1 split variable)
Then: ∇²v appears directly in weak form
```

---

### Long-Term Goals (3-6 Months)

#### 7. Shape Propagation & Type System [MEDIUM PRIORITY]
**Effort**: 1 week

**Goal**: Properly track scalar/vector/tensor types through pipeline

**Current**: Splitting assumes scalar original variables

**Improvements**:
- Implement `computeSplitVariableShape()` (currently stub)
- Rules:
  - grad(scalar) → vector
  - grad(vector) → rank-2 tensor
  - div(vector) → scalar
  - div(tensor) → vector
- Validate shape consistency throughout AST
- Better error messages for shape mismatches

#### 8. Nonlocal Operators [HIGH PRIORITY - New Capability]
**Effort**: 2-3 weeks

**Goal**: Support integral terms in energy functionals

**Use cases**:
- Nonlocal phase-field models
- Peridynamics
- Fractional derivatives
- Long-range interactions

**Syntax example**:
```
F = ∫∫ kernel(x,x') * u(x) * u(x') dx dx'
strong_forms = 'u = var_diff(integral(kernel(x,xp)*u(xp), xp), u)'
```

**Challenges**:
- Expensive O(N²) operations
- Requires quadrature over domain
- Special kernel classes needed

#### 9. Tensor Mechanics with Proper Strain Measures [MEDIUM PRIORITY]
**Effort**: 2 weeks

**Goal**: Support mechanics problems with correct kinematics

**Needed**:
- Green-Lagrange strain: E = ½(F^T F - I)
- Deformation gradient: F = I + ∇u
- Stress measures: PK1, PK2, Cauchy
- Objective rates

**Example**:
```
strong_forms = 'disp = var_diff(elastic_energy(GreenLagrange(disp), C), disp)'
```

#### 10. Adaptive Variable Splitting [LOW PRIORITY]
**Effort**: 2-3 weeks

**Goal**: Dynamically adjust splitting based on solution behavior

**Approach**:
- Monitor error estimates
- Refine split variable resolution in high-gradient regions
- Coarsen where smooth
- Adaptively choose split strategy (gradient vs laplacian)

---

### Research/Exploratory Goals (6+ Months)

#### 11. Boundary Integrals in Energy Functionals
**Goal**: Support surface energy terms

**Example**:
```
F = ∫_Ω f dx + ∫_∂Ω g ds
strong_forms = 'u = var_diff(bulk_energy + surface_term, u)'
```

#### 12. Time-Dependent Energy Functionals
**Goal**: Support non-conservative systems

**Example**: Dissipative terms, external driving

#### 13. Parallel Performance Optimization
**Goal**: Efficient evaluation for large systems
- Kernel fusion
- Expression caching
- Parallel assembly

#### 14. Stabilization for Complex PDEs
**Goal**: Built-in stabilization (SUPG, GLS, etc.)
- Detect convection-dominated problems
- Auto-generate stabilization terms
- Parameter estimation

---

## Implementation Strategy

### General Principles

1. **Avoid Special Cases**: Keep the architecture general
   - Use expression-based specification
   - No hard-coded physics models
   - Rely on parser and symbolic manipulation

2. **Reuse Existing Infrastructure**:
   - Use WeakFormGenerator for variational derivatives
   - Use StrongFormLowerer for IBP
   - Use MOOSE's existing kernels when possible

3. **Test-Driven Development**:
   - Add test for new feature
   - Implement feature
   - Verify test passes
   - Add regression test

4. **Backward Compatibility**:
   - Maintain existing test cases
   - Deprecate gracefully (warnings, not errors)
   - Document migration paths

---

## Contributing

### Adding New Operators

1. **Update MooseAST.h**: Add new NodeType
2. **Update StringExpressionParser.C**: Add parsing rule
3. **Update DifferentiationVisitor**: Add differentiation rule in WeakFormGenerator.C
4. **Update StrongFormLowerer.C**: Add IBP rule if applicable
5. **Add test case**: Create input file in `test/tests/variational/`
6. **Document**: Update this CLAUDE.md

### Adding New Features

1. Read relevant sections of this document
2. Check "Known Issues" to avoid duplicating work
3. Follow existing code patterns
4. Add comprehensive tests
5. Update documentation
6. Ensure backward compatibility

### Debugging New Issues

1. Enable `verbose = true` and all `debug_print_*` options
2. Check parsed AST structure
3. Verify IBP lowering produces correct c0/c1
4. Check generated kernel parameters
5. Use `--show-input` flag to see expanded input

---

## References

### Theory
- Variational principles in continuum mechanics
- Phase-field modeling literature
- Finite element weak forms
- Integration by parts techniques
- Variational derivatives and functional analysis

### Software
- MOOSE documentation: https://mooseframework.inl.gov
- MOOSE examples: https://github.com/idaholab/moose/tree/master/examples
- Automatic differentiation: https://github.com/roystgnr/MetaPhysicL
- Expression parsing: PEG parser theory

### Papers
- Phase-field methods for complex geometries
- Automatic code generation for finite elements
- Symbolic differentiation techniques
- Variable splitting methods for higher-order PDEs

---

## Quick Reference Card

### Basic Syntax
```ini
[AutomaticWeakForm]
  # Direct strong form
  strong_forms = 'u_t = D*laplacian(u) - k*u'

  # Energy functional
  strong_forms = 'u = var_diff(energy_expr, u)'

  # Multiple equations
  strong_forms = 'u_t = ...; v_t = ...'

  # Required
  variables = 'u v'

  # Optional
  parameters = 'D 1.0 k 0.1'
  use_automatic_differentiation = false  # Until AD fixed
  verbose = true
  enable_splitting = true
  max_fe_order = 1
[]
```

### Common Functions
- `grad(u)`, `div(V)`, `laplacian(u)`, `curl(V)`
- `dot(v1,v2)`, `cross(v1,v2)`, `norm(v)`
- `W(c)` = (c²-1)²
- `var_diff(energy, var)` = δF/δvar

### Getting Help
- Set `verbose = true` to see what's happening
- Check this file for examples
- Read test files in `test/tests/variational/`
- Check error messages from parser (usually helpful)

---

**Last Updated**: 2026-02-09
**Document Version**: 2.0 (Unified var_diff() architecture)
