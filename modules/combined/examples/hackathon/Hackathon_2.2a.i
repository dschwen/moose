#
# Hackathon Problem 2.2a
# units are transformed to eV and nm
#
# 3e-9 J/m = 18.7245280299 eV/nm
# 2.4e8 J/m^3 = 1.4979622423920003 eV/nm^3
# 1 GPa =6.25 eV/nm^3
# The 0.0042 um^3 volume can be achieved with a sphere of a radius of 100.089 nm
#
#

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 8
  ny = 8
  nz = 8
  xmin = -200
  xmax = 200
  ymin = -200
  ymax = 200
  zmin = -200
  zmax = 200
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[MeshModifiers]
  [./cnode]
    # center node
    type = AddExtraNodeset
    coord = '0.0 0.0 0.0'
    new_boundary = 10
  [../]
  [./snode]
    # side node
    type = AddExtraNodeset
    coord = '0.0 200.0 0.0'
    new_boundary = 11
  [../]
  [./enode]
    # edge node
    type = AddExtraNodeset
    coord = '0.0 200.0 200.0'
    new_boundary = 12
  [../]
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothSuperellipsoidIC
      x1 = 0
      y1 = 0
      z1 = 0
      a = 155.0
      b = 42.0
      c = 155.0
      n = 2
      invalue = 1.0
      outvalue = 0.0
      int_width = 10.0
    [../]
  [../]
  [./w]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_z]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./TensorMechanics]
  [../]

  [./c_res]
    type = SplitCHParsed
    variable = c
    f_name = F
    kappa_name = kappa_c
    w = w
  [../]
  [./w_res]
    type = SplitCHWRes
    variable = w
    mob_name = M
  [../]
  [./time]
    type = CoupledTimeDerivative
    variable = w
    v = c
  [../]
[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    prop_names  = 'M kappa_c'
    prop_values = '1 18.7245280299'
    block = 0
  [../]

  # simple chemical free energy with a miscibility gap
  [./chemical_free_energy]
    type = DerivativeParsedMaterial
    block = 0
    f_name = Fc
    args = 'c'
    constant_names = h0
    constant_expressions = 1.4979622423920003
    function = h0*c^2*(1-c)^2
    enable_jit = true
    derivative_order = 2
  [../]

  [./switchingfunction]
    type = SwitchingFunctionMaterial
    h_order = HIGH
    function_name = h
    eta = c
  [../]
  [./oneminus]
    type = DerivativeParsedMaterial
    material_property_names = h(c)
    f_name = ih
    function = 1-h
  [../]

  [./elasticity_tensor_potato]
    type = ComputeElasticityTensor
    block = 0
    base_name = potato
    C_ijkl = '1458.75 903.125 726.875 1806.25 508.75 2375.625 621.875 548.75 701.875'
    fill_method = symmetric9
  [../]
  [./elasticity_tensor_matrix]
    type = ComputeElasticityTensor
    block = 0
    base_name = matrix
    C_ijkl = '1e-10 1e-10'
    fill_method = symmetric_isotropic
  [../]
  [./elasticity_tensor]
    type = CompositeElasticityTensor
    tensors = 'potato matrix'
    weights = 'h      ih'
    args = c
  [../]

  [./stress]
    type = ComputeLinearElasticStress
    block = 0
  [../]
  [./strain]
    type = ComputeSmallStrain
    block = 0
  [../]

  [./elastic_free_energy]
    type = ElasticEnergyMaterial
    f_name = Fe
    block = 0
    args = 'c'
    derivative_order = 2
  [../]

  # Sum up chemical and elastic contributions
  [./free_energy]
    type = DerivativeSumMaterial
    block = 0
    f_name = F
    sum_materials = 'Fc Fe'
    args = 'c'
    derivative_order = 2
  [../]
[]

[BCs]
  # prevent translation
  [./cnode_x]
    type = PresetBC
    variable = disp_x
    boundary = 10
    value = 0
  [../]
  [./cnode_y]
    type = PresetBC
    variable = disp_y
    boundary = 10
    value = 0
  [../]
  [./cnode_z]
    type = PresetBC
    variable = disp_z
    boundary = 10
    value = 0
  [../]

  # prevent rotation around x-axis and z-axis
  [./snode_x]
    type = PresetBC
    variable = disp_x
    boundary = 11
    value = 0
  [../]
  [./snode_z]
    type = PresetBC
    variable = disp_z
    boundary = 11
    value = 0
  [../]

  # lock rotation around the y-axis
  [./enode_x]
    type = PresetBC
    variable = disp_x
    boundary = 12
    value = 0
  [../]
[]

[Postprocessors]
  [./vol]
    type = ElementIntegralVariablePostprocessor
    variable = c
    execute_on = 'initial TIMESTEP_END'
  [../]
  [./dofs]
    type = NumDOFs
    execute_on = 'initial TIMESTEP_END'
  [../]
  [./runtime]
    type = RunTime
    time_type = active
    execute_on = 'initial TIMESTEP_END'
  [../]
[]

[Preconditioning]
  # active = ' '
  [./SMP]
    type = SMP
    coupled_groups = 'c,w disp_x,disp_y,disp_z'
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2

  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type  -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm       ilu          1'

  l_max_its = 30
  nl_max_its = 10
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10
  start_time = 0.0
  num_steps = 200

  [./TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 10
  [../]

  [./Adaptivity]
    max_h_level = 4
    interval = 3
    print_changed_info = true
    initial_adaptivity = 4
    refine_fraction = 0.7
    coarsen_fraction = 0.1
  [../]
[]

[Outputs]
  exodus = true
  execute_on = 'INITIAL TIMESTEP_END'
[]
