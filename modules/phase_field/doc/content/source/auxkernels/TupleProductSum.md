# TupleProductSum

!syntax description /AuxKernels/TupleProductSum

This AuxKernel computes the sums of the form

\begin{equation}
S_k=\sum_{i_0 < N-k+1}\left( \sum_{i_1\lt i_0}\left( \sum_{i_2\lt i_1}\left( \dots \sum_{i_{(k-1)}\lt i_{(k-2)}} \left[ \prod_{j\lt k} v_{i_j} \right]\right)\right)\right),
\end{equation}
where the number of nested sum operators $k$ is given by the parameter `order`
and $N$ is the number of variables $v_i$. With a passed in vector $v$ (`v`) of
variables $v_0, v_1, v_2, v_3$ this should result in the sums
\begin{equation}
S_k = \begin{cases}
  v_0 + v_1 + v_2 + v_3  &k=1 \\
  v_0v_1 + v_0v_2 + v_0v_3 + v_1v_2 + v_1v_3 + v_2v_3  &k=2 \\
  v_0v_1v_2 + v_0v_1v_3 + v_0v_2v_3 + v_1v_2v_3  &k=3 \\
  \dots &
\end{cases}
\end{equation}

## Design

Computing the sum of the product each $k$ member subset of the $N$ variables
$v_i$ is mapped onto the problem of finding all $\binom{N}{k}$ subsets of size
$k$ in the number range $\{0,\dots, N-1\}$. These subsets are the index sets for
the vector of variables $v_i$ and are built using a recursive algorithm at the
start of the simulation.

!syntax parameters /AuxKernels/TupleProductSum

!syntax inputs /AuxKernels/TupleProductSum

!syntax children /AuxKernels/TupleProductSum

!bibtex bibliography
