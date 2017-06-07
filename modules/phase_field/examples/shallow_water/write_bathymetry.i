#
# Example problem demonstrating the implementation for the linearized shallow water equations
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 800
  ny = 440
  xmin = -180
  ymin = -90
  xmax = 180
  ymax = 90
[]

[AuxVariables]
  # bathymetry
  [./h]
  [../]
[]

[ICs]
  [./eta]
    type = LoadXYZIC
    variable = h
    file = ETOPO1_Bed_g_int.xyz
    numx = 21601
    numy = 10801
    xmin = -180
    xmax = 180.01
    ymin = -90
    ymax = 90.01
  [../]
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  exodus = true
  execute_on = initial
[]
