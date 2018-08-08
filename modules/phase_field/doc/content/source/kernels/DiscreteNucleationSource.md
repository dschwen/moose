
# DiscreteNucleationSource

!syntax description /Kernels/DiscreteNucleationSource

It implements the weak for term

\begin{equation}
(M(r) \cdot (v_1 - v_0) + v_0 - u, \psi),
\end{equation}

where $u$ ist the variable the kernel is acting on, $M$ is the
[DiscreteNucleationMap](/DiscreteNucleationMap.md)] data, and $\psi$ is a  test
function. The map values $M(r)$ range from 0..1 and are remaped using the
$v_0$ (`no_nucleus_value`) and $v_1$ (`nucleus_value`) parameters.

The `DiscreteNucleationSource` kernel is part of the [Discrete Nucleation system](Nucleation/DiscreteNucleation.md).
Its companion AuxKernel is [DiscreteNucleationAux](/DiscreteNucleationAux.md).

!syntax parameters /Kernels/DiscreteNucleationSource

!syntax inputs /Kernels/DiscreteNucleationSource

!syntax children /Kernels/DiscreteNucleationSource
