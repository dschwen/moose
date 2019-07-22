# SwitchingFunctionConstraintEta

!syntax description /Kernels/SwitchingFunctionConstraintEta

This kernel adds the term
\begin{equation}
\left(\lambda\frac{\partial k}{\partial\eta_i}, \psi\right)
\end{equation}
to the equation for order parameter $\eta_i$. Here $k$ is the constraint
\begin{equation}
k(\vec \eta)=\left[\sum_i h(\eta_i)\right] - 1 \underbrace{- \frac\epsilon2\lambda}_{\text{Jacobian fill}}.
\end{equation}

See also
[`SwitchingFunctionConstraintLagrange`](/SwitchingFunctionConstraintLagrange.md)
and [WBM multiphase model](/WBM.md).

!syntax parameters /Kernels/SwitchingFunctionConstraintEta

!syntax inputs /Kernels/SwitchingFunctionConstraintEta

!syntax children /Kernels/SwitchingFunctionConstraintEta

!bibtex bibliography
