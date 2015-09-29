[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 6000
  ny = 1
  xmin = -5
  xmax = 5
  ymin = 0
  ymax = 1
  elem_type = QUAD4
[]

[GlobalParams]
  # Output all material properties to the exodus file:
  outputs = exodus
[]

[AuxVariables]
  [./local_energy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./cross_energy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
[]

[Variables]
  [./c]
    order = THIRD
    family = HERMITE
   
    [./InitialCondition]
      type = FunctionIC
      variable = c
      function = 'if(x<-0.50,0.25,if(x<0,0.675,0.21))'
    [../]
  [../]

  [./eta]
    order = FIRST
    family = LAGRANGE
    
    [./InitialCondition]
      type = FunctionIC
      function = 'if(x<-0.50,0,if(x<0,0.5,1))'
    [../]
  [../]
[]

[Kernels]
  [./c_res]
    type = CHParsed
    variable = c
    f_name = F
    mob_name = M
    args = 'eta'
  [../]

  [./time]
    type = TimeDerivative
    variable = c
  [../]

  # The order parameter eta is acted on by 3 kernels:
  #  1. The stock time derivative deta_i/dt kernel
  #  2. The Allen-Cahn kernel that takes a Derivative Material for the free energy
  #  3. A gradient interface kernel that includes cross terms
  [./detadt]
    type = TimeDerivative
    variable = eta
  [../]

  [./ACBulk]
    type = ACParsed
    variable = eta
    args = 'eta c'
    mob_name = L
    f_name = F
  [../]

  [./ACInterface]
    type = ACInterface
    variable = eta
    mob_name = L
    kappa_name = 'kappa_material'
    args = 'c eta'
  [../]

[]

[BCs]
  [./c-left]
    type = DirichletBC
    variable = c
    boundary = 'left'
    value = 0.1
  [../]

  [./c-right]
    type = DirichletBC
    variable = c
    boundary = 'right'
    value = 0.21
  [../]

  [./eta-left]
    type = DirichletBC
    variable = eta
    boundary = 'left'
    value = 0
  [../]

  [./eta-right]
    type = DirichletBC
    variable = eta
    boundary = 'right'
    value = 1
  [../]

[]

[Materials]

active = 'consts phase_free_energy mobility-expression kappa_material'

  # Here we set some of the model parameters: the mobilities and interface
  # gradient prefactors. Based on Table 1 in Zaeem2014.
  [./consts]
    type = GenericConstantMaterial
    block = 0
    prop_names  = 'L'
    prop_values = '1e-7'
  [../]

  # This material returns the free energy of the system as a function of the
  # concentration and non-conserved order parameter eta, including the
  # interface penalty term W(phi)*g(phi) in Zaeem2014 equations 2-8. 
  [./phase_free_energy]
    type = DerivativeParsedMaterial
    block = 0
    f_name = F
    function = '2*(eta-0.5)^2*(eta-1)^2*1560549*(c-0.291)^2+
                8*eta^2*(eta-1)^2*1560549*(c-0.67)^2+
                2*eta^2*(eta-0.5)^2*1560549*(c-0.21)^2+
                (4*3.033707865e5*((eta-0.75)^2+(eta-0.25)^2))*(12945.38272*eta^4*(eta-0.5)^2*(eta-1)^2)'
                #' (4*3.033707865e9*((eta-0.75)^2+(eta-0.25)^2))*(12945.38272*eta^4*(eta-0.5)^2*(eta-1)^2)'
    args = 'c eta'
  [../]

  # This material returns the mobility of each phase, as implemented in Zaeem2014 (eqn 21).
  [./mobility-expression]
    type = DerivativeParsedMaterial
    block = 0
    f_name = M
    args = 'eta'
    function = '4*(eta-0.5)^2*(eta-1)^2*(3.5e-2)/1560549+
                16*eta^2*(eta-1)^2*(1.3e-2)/1560549+
                4*eta^2*(eta-0.5)^2*(2.4e-2)/1560549'
    outputs = exodus
    derivative_order = 3
  [../]

  # This material returns the value of kappa (Zaeem2014 eqn. 17).
  [./kappa_material]
    type = DerivativeParsedMaterial
    block = 0
    f_name = kappa_material
    args = 'eta'
    function = '4*264669*(eta-0.75)^2+4*10424*(eta-0.25)^2'
    outputs = exodus
    derivative_order = 3
  [../]

[]

[Postprocessors]
active = 'total_solute'

  # The total free enrgy of the simulation cell to observe the energy reduction.
  [./total_free_energy]
    type = ElementIntegralVariablePostprocessor
    variable = local_energy
    #outputs = 'console'
  [../]

  # for testing we also monitor the total solute amount, which should be conserved.
  [./total_solute]
    type = ElementIntegralVariablePostprocessor
    variable = c
    outputs = 'console csv'
  [../]
[]

[Preconditioning]
  # This preconditioner makes sure the Jacobian Matrix is fully populated. Our
  # kernels compute all Jacobian matrix entries.
  # This allows us to use the Newton solver below.
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  # Automatic differentiation provedes a _full_ Jacobian in this example
  # so we can safely use NEWTON for a fast solve
  solve_type = 'NEWTON'

  l_max_its = 15
  l_tol = 1.0e-6

  nl_max_its = 25 # 50
  nl_rel_tol = 1.0e-6
  nl_abs_tol = 1.0e-6

  start_time = 0.0
  end_time   = 1200.0 # 3600.0

  [./TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 0.0001
  [../]
[]

[Debug]
  # show_var_residual_norms = true
[]

[Outputs]
  interval = 1
  exodus = true
  csv = true
  output_initial = true
  print_linear_residuals = true
  [./table]
    type = CSV
    delimiter = ' '
  [../]
  print_perf_log = true
[]
