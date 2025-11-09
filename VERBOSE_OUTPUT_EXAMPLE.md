# Verbose Output Example for Automatic Weak Form System

## Overview

The automatic weak form generation system includes comprehensive debugging and verbose output options to help understand the processing steps and troubleshoot issues.

## Available Output Options

The following parameters control the verbosity of output:

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `verbose` | bool | false | Enable verbose output showing all processing steps |
| `debug_print_expressions` | bool | false | Print all parsed expressions |
| `debug_print_derivatives` | bool | false | Print derivative calculations |
| `debug_print_simplification` | bool | false | Show expression simplification steps |
| `debug_print_weak_form` | bool | false | Print final weak form |
| `debug_print_jacobian` | bool | false | Print Jacobian expressions |
| `output_weak_form` | bool | false | Write weak form to file |
| `weak_form_file` | string | "weak_form.txt" | Output file name |

## Example Input File

```ini
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 50
[]

[Variables]
  [c]
    order = FIRST
    family = LAGRANGE
  []
[]

[AutomaticWeakForm]
  [cahn_hilliard]
    # Energy functional
    energy_type = expression
    energy_expression = '(c*c - 1.0)*(c*c - 1.0) + 0.5*kappa*dot(grad(c), grad(c))'
    variables = 'c'
    parameters = 'kappa 0.01'
    
    # Enable automatic differentiation
    use_automatic_differentiation = true
    
    # Verbose output options
    verbose = true                      # Main verbose flag
    debug_print_expressions = true      # Show parsed expressions
    debug_print_derivatives = true      # Show derivative steps
    debug_print_simplification = true   # Show simplification
    debug_print_weak_form = true       # Show final weak form
    debug_print_jacobian = true        # Show Jacobian
    
    # Write to file
    output_weak_form = true
    weak_form_file = 'cahn_hilliard_debug.txt'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
```

## Expected Output

When verbose options are enabled, you'll see output like:

### 1. Processing Information (verbose=true)
```
========================================
[AutomaticWeakForm] Adding Kernels
========================================
Problem type: STEADY-STATE

Processing single energy functional

--- Processing variable: c ---

[AutomaticWeakForm] Generating kernel for variable: c
  - Kernel name: c_variational
  - Kernel type: VariationalKernelBase
  - Automatic differentiation: ENABLED
  - Kernel successfully added to problem

========================================
[AutomaticWeakForm] Kernel Addition Complete
========================================
```

### 2. Expression Processing (debug_print_expressions=true)
```
Energy: ((c*c - 1.0)*(c*c - 1.0) + (0.500000 * (0.010000 * dot(grad(c), grad(c)))))
```

### 3. Derivative Calculations (from WeakFormGenerator)
```
Differential coefficients for variable c:

  C^0 = (((2.000000 * c) * (pow(c, 2.000000) - 1.000000)) + ((pow(c, 2.000000) - 1.000000) * (2.000000 * c)))
  
  Processing C^1 = (0.005000 * (2.000000 * grad(c)))
    After divergence 1: div((0.005000 * (2.000000 * grad(c))))
    After negation: -(div((0.005000 * (2.000000 * grad(c)))))

Final Euler-Lagrange equation: ((((2.000000 * c) * (pow(c, 2.000000) - 1.000000)) + ((pow(c, 2.000000) - 1.000000) * (2.000000 * c))) + -(div((0.005000 * (2.000000 * grad(c))))))
```

### 4. Weak Form File Output (output_weak_form=true)

Contents of `cahn_hilliard_debug.txt`:
```
# Automatically generated weak form
# Energy type: expression
# Variables: c 

# Weak form for variable: c
((((2.000000 * c) * (pow(c, 2.000000) - 1.000000)) + ((pow(c, 2.000000) - 1.000000) * (2.000000 * c))) + -(div((0.005000 * (2.000000 * grad(c))))))
```

## Transient Problems

For transient problems, additional output shows time derivative kernel addition:

```ini
[Executioner]
  type = Transient
  dt = 0.01
  end_time = 1.0
[]
```

With verbose output:
```
[AutomaticWeakForm] Adding time derivative kernel for variable: c
  - Using VariationalTimeDerivative with automatic differentiation
  - Kernel name: c_time_derivative
```

## Debugging Tips

1. **Start with `verbose=true`** to see the overall flow
2. **Add `debug_print_expressions=true`** if parsing seems incorrect
3. **Enable `debug_print_weak_form=true`** to verify the final form
4. **Use `output_weak_form=true`** to save the weak form for analysis
5. **For convergence issues, enable `debug_print_jacobian=true`**

## Implementation Details

The verbose output is implemented at multiple levels:

1. **AutomaticWeakFormAction**: Controls kernel generation and provides high-level verbose output
2. **WeakFormGenerator**: Outputs derivative calculations and weak form generation steps
3. **ExpressionSimplifier**: Can output simplification steps (when enabled)
4. **VariationalKernelBase**: Evaluates the weak form at quadrature points

## Performance Note

Verbose output options are designed for debugging and should be disabled in production runs to avoid performance overhead and excessive console output.

## Future Enhancements

Planned improvements to the verbose output system:
- Color-coded output for different message types
- Structured logging with levels (ERROR, WARNING, INFO, DEBUG)
- Performance timing for each processing step
- Memory usage reporting for large expressions
- Graphical visualization of expression trees