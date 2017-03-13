# Further Explanations

## Divergence Theorem
The divergence theorem states that the volume integral of the divergence of a vector field over a volume $V$ bounded by a surface $S$ is equal to the surface integral of the vector field projected on the outward facing normal of the surface $S$.

$$
\int_\Omega \nabla F dV = \int_{\partial\Omega} F\cdot\mathbf{n}dS
$$

## Product Rule
Product rule for the product of a scalar $a$ and a vector $\mathbf{b}$ is useful to reduce the derivative order on an expression in conjunction with the divergence theorem.

$$
\nabla (a\mathbf{b}) = \nabla a \cdot \mathbf{b} + a \nabla\cdot\mathbf{b}
$$

Shuffle the terms (and note that this is valid for a vector $\mathbf{a}$ and a scalar $b$ as well)

$$
-\nabla a \cdot \mathbf{b}  =  a \nabla\cdot\mathbf{b} - \nabla (a\mathbf{b})
$$

$$
-\nabla \cdot \mathbf{a} b  = \mathbf{a}\cdot \nabla b - \nabla (\mathbf{a}b)
$$

The right most term ($\nabla(\dots)$) can be transformed using the divergence theorem. This can be used to effectively shift a derivative over to the test function when building a residual.

## Fundamental Lemma of calculus of variations
The functional derivative in the Cahn-Hilliard equation can be calculated using this rule

$$
\frac{\delta F}{\delta c} = \frac{\partial f}{\partial c} - \nabla \cdot \frac{\partial f}{\partial\nabla c}
$$

Note that the general form of this derivative for functionals containing higher order derivatives is

$$
\frac{\delta F}{\delta c} = \frac{\partial f}{\partial c} + \sum_n (-1)^n \nabla^{(n)} \cdot \frac{\partial f}{\partial\nabla^{(n)} c}
$$

## Weak form of the `ACInterface` Kernel

The term $L\nabla(\kappa\nabla\eta)$ is multiplied with the test function $\psi$ and integrated, yielding

$$
\left(L\nabla(\kappa\nabla\eta),\psi\right) = \left(\nabla \cdot \underbrace{(\kappa\nabla\eta)}_{\equiv \mathbf{a}},\underbrace{L\psi}_{\equiv b}\right)
$$

we moved the $L$ over to the right and identify a vector term $\mathbf{a}$ and a scalar term $b$. Then we use the third equality in the _Product rule_ section to obtain

$$
-\mathbf{a}\cdot \nabla b + \nabla (\mathbf{a}b).
$$

The last term is converted into a surface integral using the _Divergence theorem_, tielding

$$
- \left( \kappa\nabla\eta,\nabla(L\psi) \right)
+ \left< L\kappa \nabla\eta \cdot \vec n, \psi\right>
$$
