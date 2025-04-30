[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 100
  ny = 100
  nz = 100
[]

[Problem]
  extra_tag_matrices = 'mass'
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [Mass_x_first]
    type = MassMatrix
    variable = u
    density = 1
    matrix_tags = 'mass'
  []
  [null]
    type = NullKernel
    variable = u
  []
[]

[Executioner]
  type = Transient
  start_time = 0.0
  num_steps = 100
  dt = 0.00005
  l_tol = 1e-12
  [TimeIntegrator]
    type = ExplicitMixedOrder
    mass_matrix_tag = 'mass'
    use_constant_mass = true
    first_order_vars = 'u'
  []
[]

[Outputs]
  perf_graph = true
[]

# Performance Graph:
# --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# |                                Section                               | Calls |   Self(s)  |   Avg(s)   |    %   | Mem(MB) |  Total(s)  |   Avg(s)   |    %   | Mem(MB) |
# --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# | SolidMechanicsTestApp (main)                                         |     1 |      0.004 |      0.004 |   0.00 |       2 |    324.463 |    324.463 | 100.00 |    1918 |
# |   Action::SetupMeshAction::Mesh::SetupMeshAction::act::setup_mesh    |     1 |      0.000 |      0.000 |   0.00 |       0 |      0.000 |      0.000 |   0.00 |       0 |
# |   Action::SetupMeshAction::Mesh::SetupMeshAction::act::set_mesh_base |     2 |      0.002 |      0.001 |   0.00 |       0 |      1.278 |      0.639 |   0.39 |     352 |
# |   FEProblem::computeUserObjects                                      |     2 |      0.000 |      0.000 |   0.00 |       0 |      0.000 |      0.000 |   0.00 |       0 |
# |   FEProblem::computeUserObjects                                      |     3 |      0.000 |      0.000 |   0.00 |       0 |      0.000 |      0.000 |   0.00 |       0 |
# |   FEProblem::outputStep                                              |   101 |      0.467 |      0.005 |   0.14 |       0 |    107.935 |      1.069 |  33.27 |       0 |
# |   Transient::PicardSolve                                             |   100 |      0.079 |      0.001 |   0.02 |       0 |    196.959 |      1.970 |  60.70 |     220 |
# |     FEProblem::computeUserObjects                                    |   800 |      0.002 |      0.000 |   0.00 |       0 |      0.002 |      0.000 |   0.00 |       0 |
# |     FEProblem::outputStep                                            |   300 |      0.122 |      0.000 |   0.04 |       0 |      0.123 |      0.000 |   0.04 |       0 |
# |     FEProblem::solve                                                 |   100 |      6.574 |      0.066 |   2.03 |       1 |    196.749 |      1.967 |  60.64 |     220 |
# |       FEProblem::computeUserObjects                                  |     2 |      0.000 |      0.000 |   0.00 |       0 |      0.000 |      0.000 |   0.00 |       0 |
# |       FEProblem::computeResidualInternal                             |   100 |      0.006 |      0.000 |   0.00 |       0 |    182.771 |      1.828 |  56.33 |       0 |
# |         FEProblem::computeUserObjects                                |   200 |      0.001 |      0.000 |   0.00 |       0 |      0.001 |      0.000 |   0.00 |       0 |
# |   Transient::final                                                   |     1 |      0.000 |      0.000 |   0.00 |       0 |      0.001 |      0.001 |   0.00 |       0 |
# |     FEProblem::computeUserObjects                                    |     2 |      0.000 |      0.000 |   0.00 |       0 |      0.000 |      0.000 |   0.00 |       0 |
# |     FEProblem::outputStep                                            |     1 |      0.001 |      0.001 |   0.00 |       0 |      0.001 |      0.001 |   0.00 |       0 |
# --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# Finished Executing                                                                       [316.72 s] [ 2296 MB]
