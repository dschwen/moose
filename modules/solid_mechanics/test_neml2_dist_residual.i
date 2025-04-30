[Mesh]
  [gen]
    type = DistributedRectilinearMeshGenerator
    dim = 3
    nx = 100
    ny = 100
    nz = 100
  []
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
[]

[UserObjects]
  [neml2_residual]
    type = NEML2Residual
    variables = u
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

# Performance Graph CPU:
# --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# |                                Section                               | Calls |   Self(s)  |   Avg(s)   |    %   | Mem(MB) |  Total(s)  |   Avg(s)   |    %   | Mem(MB) |
# --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
# | SolidMechanicsTestApp (main)                                         |     1 |      0.004 |      0.004 |   0.00 |       4 |    285.711 |    285.711 | 100.00 |    1992 |
# |   Action::SetupMeshAction::Mesh::SetupMeshAction::act::setup_mesh    |     1 |      0.000 |      0.000 |   0.00 |       0 |      0.000 |      0.000 |   0.00 |       0 |
# |   Action::SetupMeshAction::Mesh::SetupMeshAction::act::set_mesh_base |     2 |      0.002 |      0.001 |   0.00 |       0 |      1.280 |      0.640 |   0.45 |     352 |
# |   FEProblem::computeUserObjects                                      |     2 |      0.000 |      0.000 |   0.00 |       0 |      0.000 |      0.000 |   0.00 |       0 |
# |   FEProblem::computeUserObjects                                      |     3 |      0.000 |      0.000 |   0.00 |       0 |      0.000 |      0.000 |   0.00 |       0 |
# |   FEProblem::outputStep                                              |   101 |      0.474 |      0.005 |   0.17 |       0 |     92.849 |      0.919 |  32.50 |       1 |
# |   Transient::PicardSolve                                             |   100 |      0.082 |      0.001 |   0.03 |       0 |    172.682 |      1.727 |  60.44 |     221 |
# |     FEProblem::computeUserObjects                                    |   800 |      0.125 |      0.000 |   0.04 |       1 |      0.125 |      0.000 |   0.04 |       1 |
# |     FEProblem::outputStep                                            |   300 |      0.138 |      0.000 |   0.05 |       0 |      0.139 |      0.000 |   0.05 |       0 |
# |     FEProblem::solve                                                 |   100 |      6.568 |      0.066 |   2.30 |       1 |    172.330 |      1.723 |  60.32 |     220 |
# |       FEProblem::computeUserObjects                                  |     2 |      0.000 |      0.000 |   0.00 |       0 |      0.000 |      0.000 |   0.00 |       0 |
# |       FEProblem::computeResidualInternal                             |   100 |      0.006 |      0.000 |   0.00 |       0 |    158.392 |      1.584 |  55.44 |       0 |
# |         FEProblem::computeUserObjects                                |   200 |      0.001 |      0.000 |   0.00 |       0 |      0.001 |      0.000 |   0.00 |       0 |
# |   Transient::final                                                   |     1 |      0.000 |      0.000 |   0.00 |       0 |      0.001 |      0.001 |   0.00 |       0 |
# |     FEProblem::computeUserObjects                                    |     2 |      0.000 |      0.000 |   0.00 |       0 |      0.000 |      0.000 |   0.00 |       0 |
# |     FEProblem::outputStep                                            |     1 |      0.001 |      0.001 |   0.00 |       0 |      0.001 |      0.001 |   0.00 |       0 |
# --------------------------------------------------------------------------------------------------------------------------------------------------------------------------
