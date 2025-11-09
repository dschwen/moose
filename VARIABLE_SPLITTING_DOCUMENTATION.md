# Automatic Variable Splitting for Higher-Order PDEs

## Overview

The automatic weak form generation system includes support for **automatic variable splitting** to handle PDEs with derivatives higher than what standard finite elements can represent. This is essential for problems like:

- **4th-order Cahn-Hilliard**: Requires 2nd derivatives (Laplacian)
- **Phase-field crystal**: Requires up to 4th derivatives
- **Thin film equations**: Requires 4th-order operators
- **Biharmonic problems**: Requires the biharmonic operator ∇⁴

## Mathematical Background

### The Problem

Standard finite element methods with C⁰ continuous elements (like LAGRANGE) can only directly handle:
- **Values**: u at nodes
- **First derivatives**: ∇u (computed from shape function gradients)
- **Second derivatives** (optional): ∇²u (only with special elements or reconstruction)

For higher-order derivatives, we need **variable splitting**.

### The Solution: Variable Splitting

For an energy functional with high-order derivatives:
```
F[u] = ∫ f(u, ∇u, ∇²u, ∇³u, ...) dx
```

We introduce auxiliary variables:
- q = ∇u (for 2nd-order terms)
- r = ∇q = ∇²u (for 3rd-order terms)
- s = ∇r = ∇³u (for 4th-order terms)

This converts the high-order PDE into a system of first-order PDEs.

## Implementation Status

### ✅ Completed Components

1. **VariableSplittingAnalyzer** (`framework/include/utils/automatic_weak_form/VariableSplitting.h`)
   - Analyzes expressions to detect maximum derivative order
   - Determines which variables need splitting
   - Generates split variable definitions

2. **Split Variable Generation**
   - Automatically creates auxiliary variables for high-order derivatives
   - Generates appropriate shapes (scalar → vector for gradients)
   - Creates constraint equations (e.g., q - ∇u = 0)

3. **Action Integration** (`AutomaticWeakFormAction`)
   - Detects when splitting is needed
   - Adds auxiliary variables to the system
   - Sets up auxiliary kernels for computing split variables
   - Provides verbose output for debugging

4. **GradientAuxKernel** 
   - Computes gradient of a variable for split variable q = ∇u
   - Handles vector auxiliary variables properly

### 🚧 Partially Implemented

1. **Expression Transformation**
   - Need to replace ∇²u with ∇q in expressions after splitting
   - Requires AST transformation pass

2. **Higher-Order Kernels**
   - HessianAuxKernel for 2nd derivatives
   - Specialized kernels for 3rd and 4th derivatives

3. **Mixed Formulation Assembly**
   - Proper weak form for the coupled system
   - Saddle-point structure handling

## Usage

### Basic Example: Forcing Variable Splitting

```ini
[AutomaticWeakForm]
  [problem]
    energy_type = expression
    energy_expression = '0.5*dot(grad(u), grad(u))'
    variables = 'u'
    
    # Force splitting by limiting FE order
    enable_splitting = true
    max_fe_order = 0  # Pretend we can't compute gradients
    
    # Enable verbose output
    verbose = true
  []
[]
```

### Advanced Example: 4th-Order Cahn-Hilliard

```ini
[AutomaticWeakForm]
  [cahn_hilliard_4th]
    energy_type = expression
    # Energy with biharmonic regularization
    energy_expression = 'W(c) + 0.5*kappa*dot(grad(c), grad(c)) + 0.5*lambda*laplacian(c)^2'
    variables = 'c'
    parameters = 'kappa 0.01 lambda 0.001'
    
    # Enable automatic splitting
    enable_splitting = true
    max_fe_order = 1  # Standard elements only provide gradients
    
    verbose = true
  []
[]
```

This would automatically generate:
- Primary variable: `c`
- Auxiliary variable: `c_grad` (vector) = ∇c
- Auxiliary variable: `c_laplacian` (scalar) = ∇·c_grad

## How It Works

### Step 1: Expression Analysis

The `VariableSplittingAnalyzer` traverses the expression AST to find the maximum derivative order:

```cpp
// In VariableSplittingAnalyzer::analyzeNode
case NodeType::Gradient:
  order++; // Increase derivative order
  analyzeNode(operand, max_orders, order);
  break;
  
case NodeType::Divergence:
  order++; // Divergence also increases order
  analyzeNode(operand, max_orders, order);
  break;
```

### Step 2: Split Variable Generation

For each variable requiring splitting:

```cpp
SplitVariable sv;
sv.name = variable_name + "_grad";
sv.original_variable = variable_name;
sv.derivative_order = 1;
sv.definition = gradient(field_variable(variable_name));
sv.shape = VectorShape{dim};
```

### Step 3: Auxiliary Variable Addition

The action adds auxiliary variables during the `add_aux_variable` task:

```cpp
for (const auto & [name, sv] : _split_variables) {
  InputParameters params = _factory.getValidParams("MooseVariable");
  params.set<MooseEnum>("family") = "LAGRANGE";
  params.set<MooseEnum>("order") = "FIRST";
  _problem->addAuxVariable("MooseVariable", sv.name, params);
}
```

### Step 4: Auxiliary Kernel Creation

For computing the split variables:

```cpp
// Add GradientAuxKernel to compute q = ∇u
InputParameters params = _factory.getValidParams("GradientAuxKernel");
params.set<AuxVariableName>("variable") = "u_grad";
params.set<std::vector<VariableName>>("coupled_variable") = {"u"};
_problem->addAuxKernel("GradientAuxKernel", kernel_name, params);
```

### Step 5: Expression Transformation

The expressions are transformed to use split variables:

```
Original: f = ... + λ|∇²c|²
After splitting: f = ... + λ|∇q|² where q = ∇c
```

## Current Limitations

1. **Laplacian operator**: Not yet parsed (use `div(grad(u))` instead)
2. **Automatic transformation**: Expressions aren't automatically rewritten yet
3. **Higher than 2nd order**: Only gradient splitting is fully implemented
4. **Vector problems**: Splitting for vector fields needs more work
5. **Performance**: Split systems are larger and may be slower

## Testing

### Test 1: Force Splitting on Simple Problem
File: `test_splitting_simple.i`
- Uses simple diffusion but forces splitting with `max_fe_order = 0`
- Verifies the infrastructure works

### Test 2: 4th-Order Problem (Future)
File: `fourth_order_splitting.i`
- True 4th-order problem requiring splitting
- Will work once expression transformation is complete

## Future Enhancements

### Near-term
1. Complete expression transformation after splitting
2. Add Laplacian parsing support
3. Implement HessianAuxKernel for true 2nd derivatives
4. Add integration tests with MMS

### Long-term
1. Automatic order reduction for any PDE
2. Optimal splitting strategies (minimize auxiliary variables)
3. Direct support for C¹ elements (Hermite, etc.)
4. Specialized solvers for saddle-point systems
5. Performance optimization for split systems

## Mathematical Example: 4th-Order Cahn-Hilliard

### Original Form
Energy:
```
F[c] = ∫ [W(c) + κ/2|∇c|² + λ/2|∇²c|²] dx
```

Variational derivative:
```
δF/δc = W'(c) - κ∇²c + λ∇⁴c
```

### After Splitting
Introduce q = ∇c, giving:
```
F[c,q] = ∫ [W(c) + κ/2|q|² + λ/2|∇·q|²] dx
```

System of equations:
```
δF/δc = W'(c) - κ∇·q + λ∇²(∇·q) = 0
q - ∇c = 0 (constraint)
```

This converts the 4th-order PDE into a coupled system of 2nd-order PDEs.

## Code Architecture

```
VariableSplitting System
├── Analysis Phase
│   ├── VariableSplittingAnalyzer
│   │   ├── analyzeExpression()
│   │   ├── requiresSplitting()
│   │   └── generateSplitVariables()
│   └── SplitVariable struct
│       ├── name
│       ├── original_variable
│       ├── derivative_order
│       └── definition
├── Generation Phase
│   ├── AutomaticWeakFormAction
│   │   ├── setupVariableSplitting()
│   │   ├── addSplitVariables()
│   │   └── addAuxKernels()
│   └── GradientAuxKernel
└── Transformation Phase (TODO)
    ├── ExpressionTransformer
    └── WeakFormAdapter
```

## Summary

The automatic variable splitting system is **mostly implemented** with the core infrastructure in place:
- ✅ Detection of high-order derivatives
- ✅ Generation of split variables
- ✅ Addition of auxiliary variables
- ✅ Creation of gradient computation kernels
- ⚠️ Expression transformation (partial)
- ⚠️ Full weak form assembly (partial)

The system can detect when splitting is needed and set up the auxiliary variables, but the final step of transforming the expressions to use the split variables needs completion for production use.