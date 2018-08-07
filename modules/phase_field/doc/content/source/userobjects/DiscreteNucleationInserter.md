# DiscreteNucleationInserter

!syntax description /UserObjects/DiscreteNucleationInserter

The inserter manages the global list of currently active nucleus stabilization
sites and keeps track if any changes to the nucleus list occurred in the current
timestep. This user object takes two parameters:

- `hold_time` - the duration in time for which a stabilization site remains active
- `nucleation_rate` - a material property containing a nucleation rate density $J$. This
  material property can be calculated using classical nucleation theory for example.

The inserter linearizes the Poison probability $p$ for at least one nucleaus to appear in a
volume $V$ during the time interval of length $\Delta t$

\begin{equation}
  p = 1 - e^{-JV\Delta t},
\end{equation}

which is the inverse probability of no event occuring, to

\begin{equation}
  \tilde p = -JV\Delta t
\end{equation}

which is valid for *small* $J$, which results in a Poisson distribution where
the majority of non-zero events are $k=1$ single events. Here *small*
encompasses the entire range of rates up to rates that conver the entire
simulation volume with nuclei within a single timestep. As $\tilde p$ gets
sampled with high frequency the linearized expression results in faster
execution.

The `DiscreteNucleationInserter` is part of the [Discrete Nucleation system](Nucleation/DiscreteNucleation.md).

!syntax parameters /UserObjects/DiscreteNucleationInserter

!syntax inputs /UserObjects/DiscreteNucleationInserter

!syntax children /UserObjects/DiscreteNucleationInserter
