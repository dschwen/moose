#
# Test the DiscreteNucleation system using teh DiscreteNucleationSource kernel
# in a grain growth simulation using teh GrainTracker
#

[Mesh]
  # Mesh block.  Meshes can be read in or automatically generated
  type = GeneratedMesh
  dim = 2 # Problem dimension
  nx = 200 # Number of elements in the x-direction
  ny = 200 # Number of elements in the y-direction
  xmin = 0    # minimum x-coordinate of the mesh
  xmax = 1000 # maximum x-coordinate of the mesh
  ymin = 0    # minimum y-coordinate of the mesh
  ymax = 1000 # maximum y-coordinate of the mesh
  elem_type = QUAD4 # Type of elements used in the mesh
  parallel_type = replicated
[]

[GlobalParams]
  # Parameters used by several kernels that are defined globally to simplify input file
  op_num = 8 # Number of grains
  var_name_base = gr # Base name of grains
[]

[Variables]
  # Variable block, where all variables in the simulation are declared
  [./PolycrystalVariables]
  [../]
[]

[AuxVariables]
  [./bnds]
  [../]
[]

[Kernels]
  # Kernel block, where the kernels defining the residual equations are set up.
  [./PolycrystalKernel]
    op_num = 7 # Number of grains to put Grain-Growth equations on (ignore the top reserved OP)
    # Custom action creating all necessary kernels for grain growth.  All input parameters are up in GlobalParams
  [../]
  [./dt]
    type = TimeDerivative
    variable = gr7
  [../]
  [./nucleation]
    type = DiscreteNucleationSource
    variable = gr7
    map = map
  [../]
[]

[AuxKernels]
  # AuxKernel block, defining the equations used to calculate the auxvars
  [./bnds_aux]
    # AuxKernel that calculates the GB term
    type = BndsCalcAux
    variable = bnds
    execute_on = timestep_end
  [../]
[]

[BCs]
  # Boundary Condition block
  [./Periodic]
    [./top_bottom]
      auto_direction = 'x y' # Makes problem periodic in the x and y directions
    [../]
  [../]
[]

[Materials]
  [./CuGrGr]
    # Material properties
    type = GBEvolution # Quantitative material properties for copper grain growth.  Dimensions are nm and ns
    GBmob0 = 2.5e-6 #Mobility prefactor for Cu from Schonfelder1997
    GBenergy = 0.708 #GB energy for Cu from Schonfelder1997
    Q = 0.23 #Activation energy for grain growth from Schonfelder 1997
    T = 450 # K   #Constant temperature of the simulation (for mobility calculation)
    wGB = 14 # nm      #Width of the diffuse GB
  [../]
  [./probability]
    # This is a made up toy nucleation rate it should be replaced by
    # classical nucleation theory in a real simulation.
    type = ParsedMaterial
    f_name = P
    args = 'bnds'
    # nucleate if no grains are present yet
    function = '(1-bnds) * 3e-7'
    outputs = exodus
  [../]
[]

[UserObjects]
  [./inserter]
    # The inserter runs at the end of each time step to add nucleation events
    # that happend during the timestep (if it converged) to the list of nuclei
    type = DiscreteNucleationInserter
    hold_time = 0
    probability = P
  [../]
  [./map]
    # The map UO runs at the beginning of a timestep and generates a per-element/qp
    # map of nucleus locations. The map is only regenerated if the mesh changed or
    # the list of nuclei was modified.
    # The map converts the nucleation points into finite area objects with a given radius.
    type = DiscreteNucleationMap
    radius = 20
    int_width = 7
    periodic = gr0
    inserter = inserter
  [../]
  [./grain_tracker]
    type = GrainTracker
    threshold = 0.2
    connecting_threshold = 0.08
    compute_halo_maps = true # Only necessary for displaying HALOS
    reserve_op = 1
    reserve_op_threshold = 0.9

  [../]
[]

[Postprocessors]
  # Scalar postprocessors
  [./dt]
    # Outputs the current time step
    type = TimestepSize
  [../]
  [./nnuc]
    type = DiscreteNucleationData
    inserter = inserter
  [../]
[]

[Executioner]
  type = Transient # Type of executioner, here it is transient with an adaptive time step

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart -mat_mffd_type'
  petsc_options_value = 'hypre    boomeramg      101                ds'

  l_max_its = 30 # Max number of linear iterations
  l_tol = 1e-4 # Relative tolerance for linear solves
  nl_max_its = 40 # Max number of nonlinear iterations
  nl_abs_tol = 1e-11 # Relative tolerance for nonlienar solves
  nl_rel_tol = 1e-8 # Absolute tolerance for nonlienar solves

  start_time = 0.0
  end_time = 4000

  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 1 # Initial time step.  In this simulation it changes.
    optimal_iterations = 6 #Time step will adapt to maintain this number of nonlinear iterations
  [../]
  dtmax = 10
[]

[Outputs]
  exodus = true
  csv = true
  [./console]
    type = Console
    max_rows = 20
  [../]
[]
