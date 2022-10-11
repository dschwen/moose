#
# KKS toy problem in the split form
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 50
  nz = 0
  xmin = -3
  xmax = 3
  ymin = -3
  ymax = 3
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  # order parameter
  [eta]
    order = FIRST
    family = LAGRANGE
  []

  # hydrogen concentration
  [c]
    order = FIRST
    family = LAGRANGE
  []

  # chemical potential
  [w]
    order = FIRST
    family = LAGRANGE
  []

  # hydrogen phase concentration (matrix)
  [cm]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.0
  []
[]

[AuxVariables]
  [Fglobal]
    order = CONSTANT
    family = MONOMIAL
  []

  # hydrogen phase concentration (delta phase)
  [cd]
    initial_condition = 0.6
  []
[]

[ICs]
  [eta]
    variable = eta
    type = SmoothCircleIC
    x1 = 0.0
    y1 = 0.0
    radius = 1.25
    invalue = 1
    outvalue = 0.1
    int_width = 0.75
  []
  [c]
    variable = c
    type = SmoothCircleIC
    x1 = 0.0
    y1 = 0.0
    radius = 1.25
    invalue = 0.6
    outvalue = 0.15
    int_width = 0.75
  []
[]

[BCs]
  [Periodic]
    [all]
      variable = 'eta w c cm'
      auto_direction = 'x y'
    []
  []
[]

[Materials]
  # Free energy of the matrix
  [fm]
    type = DerivativeParsedMaterial
    f_name = fm
    args = 'cm'
    function = '(0.1-cm)^2'
  []

  # Free energy of the delta phase
  [fd]
    type = GenericConstantMaterial
    prop_names = fd
    prop_values = -0.2
  []

  # h(eta)
  [h_eta]
    type = SwitchingFunctionMaterial
    h_order = HIGH
    eta = eta
    # outputs = exodus
    # output_properties = h
  []

  # g(eta)
  [g_eta]
    type = BarrierFunctionMaterial
    g_order = SIMPLE
    eta = eta
  []

  # constant properties
  [constants]
    type = GenericConstantMaterial
    prop_names = 'M   L   kappa'
    prop_values = '0.7 0.7 0.1 '
  []
[]

[Kernels]
  # full transient
  active = 'PhaseConc CHBulk ACBulkF ACBulkC ACInterface dcdt detadt ckernel'

  # enforce c = (1-h(eta))*cm + h(eta)*cd
  [PhaseConc]
    type = KKSPhaseConcentration
    variable = cm
    cb = cd
    c = c
    eta = eta
  []

  #
  # Cahn-Hilliard Equation
  #
  [CHBulk]
    type = KKSSplitCHCRes
    variable = c
    ca = cm
    fa_name = fm
    w = w
  []

  [dcdt]
    type = CoupledTimeDerivative
    variable = w
    v = c
  []
  [ckernel]
    type = SplitCHWRes
    mob_name = M
    variable = w
  []

  #
  # Allen-Cahn Equation
  #
  [ACBulkF]
    type = KKSACBulkF
    variable = eta
    fa_name = fm
    fb_name = fd
    args = 'cm cd'
    w = 1
  []
  [ACBulkC]
    type = KKSACBulkC
    variable = eta
    ca = cm
    cb = cd
    fa_name = fm
  []
  [ACInterface]
    type = ACInterface
    variable = eta
    kappa_name = kappa
  []
  [detadt]
    type = TimeDerivative
    variable = eta
  []
[]

[Postprocessors]
  [c_min]
    type = ElementExtremeValue
    variable = c
    value_type = min
  []
  [c_max]
    type = ElementExtremeValue
    variable = c
    value_type = max
  []
  [l_it]
    type = NumLinearIterations
  []
  [nl_it]
    type = NumNonlinearIterations
  []
  [walltime]
    type = PerfGraphData
    data_type = TOTAL
    section_name = Root
  []
[]

[AuxKernels]
  [GlobalFreeEnergy]
    variable = Fglobal
    type = KKSGlobalFreeEnergy
    fa_name = fm
    fb_name = fd
    w = 1
  []
[]

[Executioner]
  type = Transient
  solve_type = PJFNK

  petsc_options_iname = '-pctype -sub_pc_type -sub_pc_factor_shift_type -pc_factor_shift_type'
  petsc_options_value = ' asm    lu          nonzero                    nonzero'

  nl_abs_tol = 1e-9
  nl_rel_tol = 1e-9

  l_max_its = 100
  nl_max_its = 100

  end_time = 1e3

  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 8
    iteration_window = 2
    growth_factor = 1.5
    dt = 0.1
  []
[]

#
# Precondition using handcoded off-diagonal terms
#
[Preconditioning]
  [full]
    type = SMP
    full = true
  []
[]

[Outputs]
  exodus = true
  csv = true
  print_linear_residuals = false
[]
