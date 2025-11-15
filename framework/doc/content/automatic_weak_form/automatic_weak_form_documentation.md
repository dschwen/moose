# Automatic Weak Form Generation System - Complete Documentation

## Table of Contents
1. [Overview](#overview)
2. [Mathematical Foundation](#mathematical-foundation)
3. [System Architecture](#system-architecture)
4. [Usage Guide](#usage-guide)
5. [Input File Syntax](#input-file-syntax)
6. [Advanced Features](#advanced-features)
7. [Debugging and Troubleshooting](#debugging-and-troubleshooting)
8. [Examples](#examples)
9. [API Reference](#api-reference)

## Overview

The Automatic Weak Form Generation System is a powerful MOOSE extension that automatically derives weak form finite element formulations from energy functionals or strong form PDEs. This system eliminates the need to manually implement residual and Jacobian calculations, ensuring mathematical consistency and reducing development time.

### Key Features
- **Automatic Variational Derivatives**: Computes δF/δu symbolically from energy functionals
- **Symbolic Differentiation**: Generates exact Jacobians using automatic differentiation
- **Expression Simplification**: Optimizes symbolic expressions for numerical efficiency
- **Higher-Order Support**: Handles up to 4th-order derivatives with automatic variable splitting
- **Multi-Physics Coupling**: Supports coupled systems with off-diagonal Jacobians
- **Time-Dependent Problems**: Automatically handles transient terms

## Mathematical Foundation

### Variational Derivatives

For an energy functional:
```
F[u] = ∫_Ω f(x, u, ∇u, ∇²u, ...) dx
```

The variational derivative (functional derivative) is:
```
δF/δu = Σ_{k=0}^m (-1)^k ∇^(k) · ∂f/∂(∇^(k)u)
```

Where:
- k=0: Direct dependence on field value u
- k=1: Dependence on gradient ∇u (requires -div of the partial)
- k=2: Dependence on Hessian ∇²u (requires +div² of the partial)

### Weak Form Generation

The strong form PDE `δF/δu = 0` is converted to weak form by:
1. Multiplying by test function ψ
2. Integrating over domain Ω
3. Applying integration by parts

Result: `∫_Ω (C^0·ψ - C^1·∇ψ + C^2:∇²ψ - ...) dx = 0`

Where C^k are the differential coefficients from the variational derivative.

## System Architecture

### Core Components

```
framework/
├── include/
│   ├── actions/
│   │   └── AutomaticWeakFormAction.h         # Main action coordinating system
│   ├── kernels/
│   │   ├── VariationalKernelBase.h           # Base kernel for weak form evaluation
│   │   └── VariationalTimeDerivative.h       # Time derivative for transient problems
│   └── utils/automatic_weak_form/
│       ├── MooseAST.h                        # Abstract syntax tree for expressions
│       ├── ExpressionParser.h                # Parse string expressions to AST
│       ├── ExpressionSimplifier.h            # Simplify symbolic expressions
│       ├── WeakFormGenerator.h               # Generate weak form from energy
│       ├── VariableSplitting.h              # Handle higher-order derivatives
│       └── MooseValueTypes.h                # Type system for symbolic math
└── src/
    └── [corresponding .C files]
```

### Processing Pipeline

1. **Parsing**: String expressions → AST
2. **Differentiation**: Energy functional → Variational derivative
3. **Simplification**: Complex expressions → Optimized forms
4. **Weak Form**: Strong form → FEM residual
5. **Assembly**: Generate MOOSE kernels

## Usage Guide

### Basic Usage

#### 1. Energy Functional Approach
```ini
[AutomaticWeakForm]
  [problem_name]
    energy_type = expression
    energy_expression = 'W(c) + 0.5*kappa*dot(grad(c), grad(c))'
    variables = 'c'
    use_automatic_differentiation = true
    verbose = true  # Enable detailed output
  []
[]
```

#### 2. Strong Form Approach
```ini
[AutomaticWeakForm]
  [problem_name]
    energy_type = strong_form
    strong_form_expression = 'dt(u) - div(D*grad(u)) + R*u = f'
    variables = 'u'
    parameters = 'D 1.0 R 0.1'
  []
[]
```

### Supported Expressions

#### Basic Operations
- Addition/Subtraction: `a + b`, `a - b`
- Multiplication/Division: `a * b`, `a / b`
- Power: `a^n`, `pow(a, n)`
- Negation: `-a`

#### Differential Operators
- Gradient: `grad(u)` - First spatial derivative
- Divergence: `div(v)` - Divergence of vector field
- Laplacian: `laplacian(u)` - Second derivative ∇²u

#### Vector Operations
- Dot product: `dot(a, b)` - Inner product
- Norm: `norm(v)` - Euclidean norm |v|
- Normalize: `normalize(v)` - Unit vector v/|v|

#### Special Functions
- Double-well: `W(c)` - Returns (c²-1)²
- Logarithm: `log(x)`
- Exponential: `exp(x)`
- Trigonometric: `sin(x)`, `cos(x)`

## Input File Syntax

### Complete Input File Structure

```ini
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 100
[]

[Variables]
  [c]
    order = FIRST
    family = LAGRANGE
  []
[]

[AutomaticWeakForm]
  [cahn_hilliard]
    # Required parameters
    energy_type = expression
    energy_expression = '(c*c - 1.0)*(c*c - 1.0) + 0.5*kappa*dot(grad(c), grad(c))'
    variables = 'c'
    
    # Optional parameters
    parameters = 'kappa 0.01'                    # Material parameters
    use_automatic_differentiation = true         # Use AD for Jacobian
    verbose = true                               # Enable detailed output
    debug_print_expressions = true               # Print all intermediate expressions
    output_weak_form = true                     # Write weak form to file
    weak_form_file = 'weak_form.txt'           # Output file name
    
    # Advanced options
    enable_variable_splitting = true            # Auto-split for high-order derivatives
    split_threshold = 2                         # Max derivative order before splitting
    compute_jacobian_numerically = false        # Use finite differences
    fd_eps = 1e-6                              # Finite difference epsilon
    
    # Optimization
    simplify_expressions = true                 # Enable expression simplification
    cache_evaluations = true                   # Cache repeated subexpressions
  []
[]

[BCs]
  # Boundary conditions as usual
[]

[Executioner]
  type = Steady  # or Transient
  solve_type = NEWTON
  
  # For transient problems, time derivatives are added automatically
[]

[Outputs]
  exodus = true
[]
```

## Advanced Features

### 1. Automatic Variable Splitting

For higher-order derivatives beyond FE capabilities:

```ini
[AutomaticWeakForm]
  [fourth_order]
    energy_expression = 'W(c) + kappa/2*|grad(c)|^2 + lambda/2*|grad(grad(c))|^2'
    variables = 'c'
    enable_variable_splitting = true
    # Automatically creates auxiliary variable q = ∇c
    # Transforms ∇²c → ∇q in expressions
  []
[]
```

### 2. Multi-Variable Systems

```ini
[AutomaticWeakForm]
  [coupled_system]
    energy_type = multiple
    variables = 'c mu'
    
    # Separate energy for each variable
    energy_expressions = 'c_energy; mu_energy'
    c_energy = 'W(c) + 0.5*kappa*dot(grad(c), grad(c))'
    mu_energy = '0.5*dot(grad(mu), grad(mu)) - mu*dW_dc(c)'
  []
[]
```

### 3. Time-Dependent Problems

```ini
[AutomaticWeakForm]
  [transient_problem]
    energy_expression = 'W(u) + 0.5*dot(grad(u), grad(u))'
    variables = 'u'
    # Time derivatives are added automatically for Transient executioner
    time_derivative_coefficient = 1.0  # Coefficient for ∂u/∂t term
  []
[]
```

### 4. Custom Material Properties

```ini
[AutomaticWeakForm]
  [with_materials]
    energy_expression = 'D(x)*dot(grad(u), grad(u))'
    variables = 'u'
    material_properties = 'D'  # Will couple to material property D
  []
[]
```

## Debugging and Troubleshooting

### Verbose Output Options

Enable detailed debugging information:

```ini
[AutomaticWeakForm]
  [problem]
    # ... other parameters ...
    
    # Debugging options
    verbose = true                      # General verbose output
    debug_print_expressions = true      # Print all symbolic expressions
    debug_print_derivatives = true      # Print derivative calculations
    debug_print_simplification = true   # Show simplification steps
    debug_print_weak_form = true       # Print final weak form
    debug_print_jacobian = true        # Print Jacobian expressions
    
    # Output to file
    output_weak_form = true
    weak_form_file = 'debug_output.txt'
  []
[]
```

### Common Issues and Solutions

#### 1. Convergence Problems

**Symptom**: Newton iterations diverge or oscillate

**Solutions**:
- Enable automatic scaling: `automatic_scaling = true`
- Check initial conditions match boundary conditions
- Verify energy functional is convex
- Use line search: `line_search = 'bt'`

#### 2. Expression Parsing Errors

**Symptom**: "Unknown function" or "Parse error"

**Solutions**:
- Check expression syntax (use `*` for multiplication)
- Verify all variables are declared
- Use parentheses to clarify precedence
- Check function names (case-sensitive)

#### 3. Jacobian Accuracy

**Symptom**: Poor convergence with AD Jacobian

**Solutions**:
- Enable expression simplification
- Check for numerical issues (division by small numbers)
- Compare with finite difference Jacobian
- Verify all coupled variables are included

### Performance Optimization

1. **Expression Simplification**: Always enabled by default
2. **Caching**: Reuse computed subexpressions
3. **Vectorization**: Use dot products instead of component-wise operations
4. **Sparse Operations**: System automatically identifies zero contributions

## Examples

### Example 1: Allen-Cahn Equation

```ini
[AutomaticWeakForm]
  [allen_cahn]
    energy_type = expression
    energy_expression = '0.25*(c*c - 1.0)*(c*c - 1.0) + 0.5*eps*eps*dot(grad(c), grad(c))'
    variables = 'c'
    parameters = 'eps 0.1'
    use_automatic_differentiation = true
  []
[]
```

### Example 2: Cahn-Hilliard Equation

```ini
[Variables]
  [c]   # Concentration
  []
  [mu]  # Chemical potential
  []
[]

[AutomaticWeakForm]
  [cahn_hilliard]
    energy_type = coupled
    variables = 'c mu'
    
    # Split formulation
    c_equation = 'dt(c) - div(M*grad(mu)) = 0'
    mu_equation = 'mu - dW_dc(c) + kappa*laplacian(c) = 0'
    
    parameters = 'M 1.0 kappa 0.01'
  []
[]
```

### Example 3: Phase-Field Crystal

```ini
[AutomaticWeakForm]
  [pfc]
    energy_type = expression
    energy_expression = '0.5*psi*(r + (1 + laplacian(psi))^2)*psi + 0.25*psi^4'
    variables = 'psi'
    parameters = 'r -0.3'
    enable_variable_splitting = true  # For 4th-order term
  []
[]
```

### Example 4: Coupled Mechanics-Diffusion

```ini
[AutomaticWeakForm]
  [coupled_mechanics]
    energy_type = multiple
    variables = 'disp_x disp_y c'
    
    # Total energy = elastic + chemical + coupling
    elastic_energy = '0.5*lambda*trace(eps)^2 + mu*inner(eps, eps)'
    chemical_energy = 'W(c) + 0.5*kappa*dot(grad(c), grad(c))'
    coupling_energy = 'alpha*c*trace(eps)'
    
    total_energy = 'elastic_energy + chemical_energy + coupling_energy'
    
    parameters = 'lambda 100 mu 50 kappa 0.01 alpha 10'
  []
[]
```

## API Reference

### AutomaticWeakFormAction Parameters

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `energy_type` | MooseEnum | required | Type: expression, strong_form, coupled, multiple |
| `energy_expression` | string | - | Energy functional as string |
| `strong_form_expression` | string | - | Strong form PDE as string |
| `variables` | string | required | Space-separated list of variables |
| `parameters` | string | - | Space-separated name-value pairs |
| `use_automatic_differentiation` | bool | false | Use AD for Jacobian |
| `verbose` | bool | false | Enable verbose output |
| `debug_print_expressions` | bool | false | Print all expressions |
| `simplify_expressions` | bool | true | Simplify symbolic expressions |
| `enable_variable_splitting` | bool | false | Auto-split high-order derivatives |
| `split_threshold` | unsigned | 2 | Max derivative order before splitting |
| `compute_jacobian_numerically` | bool | false | Use finite differences |
| `fd_eps` | Real | 1e-6 | Finite difference epsilon |
| `output_weak_form` | bool | false | Write weak form to file |
| `weak_form_file` | string | "weak_form.txt" | Output file name |

### Expression Functions

| Function | Syntax | Description |
|----------|--------|-------------|
| Gradient | `grad(u)` | Spatial gradient ∇u |
| Divergence | `div(v)` | Divergence ∇·v |
| Laplacian | `laplacian(u)` | Laplacian ∇²u |
| Dot Product | `dot(a,b)` | Inner product a·b |
| Norm | `norm(v)` | Euclidean norm \|v\| |
| Normalize | `normalize(v)` | Unit vector v/\|v\| |
| Power | `pow(a,n)` or `a^n` | Power function |
| Double-well | `W(c)` | (c²-1)² |
| Derivative | `dW_dc(c)` | 4c(c²-1) |

### Debugging Output Levels

| Level | Flag | Information Shown |
|-------|------|-------------------|
| 0 | None | Errors only |
| 1 | `verbose=true` | Basic processing steps |
| 2 | `debug_print_expressions=true` | + All expressions |
| 3 | `debug_print_derivatives=true` | + Derivative calculations |
| 4 | `debug_print_simplification=true` | + Simplification steps |
| 5 | `debug_print_weak_form=true` | + Final weak form |
| 6 | `debug_print_jacobian=true` | + Jacobian expressions |

## Best Practices

1. **Start Simple**: Begin with known problems to verify setup
2. **Use Verbose Mode**: Enable during development for debugging
3. **Check Mathematics**: Verify energy functional is well-posed
4. **Test Convergence**: Compare with analytical solutions when available
5. **Profile Performance**: Use simplification for complex expressions
6. **Document Energy**: Include physical meaning in comments

## Limitations

1. **Derivative Order**: Maximum 4th-order derivatives (with splitting)
2. **Nonlocal Terms**: No integral operators in energy functional
3. **Tensor Operations**: Limited to rank-2 tensors
4. **Custom Functions**: Must be registered in C++ code
5. **Boundary Terms**: Natural BCs only (no essential BC from energy)

## Future Enhancements

- [ ] Support for integral/nonlocal operators
- [ ] Automatic symmetry detection
- [ ] GPU acceleration for evaluation
- [ ] Symbolic linearization for Newton method
- [ ] Integration with MOOSE's AD system
- [ ] Support for variational inequalities
- [ ] Automatic preconditioner selection
- [ ] Energy minimization solvers

## References

1. Variational Methods in Materials Science
2. The Finite Element Method (Hughes)
3. Phase-Field Methods in Materials Science (Provatas & Elder)
4. MOOSE Framework Documentation

## Support

For questions, bug reports, or feature requests:
- GitHub Issues: [moose-automatic-weak-form/issues]
- MOOSE Discussions: [Tagged with 'variational']
- Email: moose-users@inl.gov

---

*Last Updated: 2024*
*Version: 1.0.0*