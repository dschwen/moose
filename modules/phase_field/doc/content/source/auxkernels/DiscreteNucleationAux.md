# DiscreteNucleationAux

!syntax description /AuxKernels/DiscreteNucleationAux

The map values $M(r)$ range from 0..1 and can be remaped using the $v_0$
(`no_nucleus_value`) and $v_1$ (`nucleus_value`) parameters as

\begin{equation}
v = M(r) \cdot (v_1 - v_0) + v_0
\end{equation}

!syntax parameters /AuxKernels/DiscreteNucleationAux

!syntax inputs /AuxKernels/DiscreteNucleationAux

!syntax children /AuxKernels/DiscreteNucleationAux

!bibtex bibliography
