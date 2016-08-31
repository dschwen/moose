[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./element_l2_error]
    # Aux field variable representing the L2 error on each element
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./exact_fn]
    type = ParsedFunction
    value = sin(2*pi*x)*sin(2*pi*y)
  [../]

  [./forcing_fn]
    type = ParsedFunction
    value = 8*pi^2*sin(2*pi*x)*sin(2*pi*y)
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./ffn]
    type = UserForcingFunction
    variable = u
    function = forcing_fn
  [../]
[]

[AuxKernels]
  [./error_aux]
    type = ElementL2ErrorFunctionAux
    variable = element_l2_error
    function = exact_fn
    solution_variable = u
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = 'bottom left right top'
    function = exact_fn
  [../]
[]

[Postprocessors]
  [./L2_errror]
    # The L2 norm of the error over the entire mesh.  Note: this is
    # *not* equal to the sum over all the elements of the L2-error
    # norms.
    type = ElementL2Error
    variable = u
    function = exact_fn
  [../]
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

