[Mesh]
  [circle]
    type = ConcentricCircleMeshGenerator
    has_outer_square = false
    num_sectors = 30
    rings = 1
    radii = 1
    preserve_volumes = false
  []
  [boundary]
    type = SideSetsAroundSubdomainGenerator
    input = circle
    new_boundary = outer
    block = 1
  []
[]

[GlobalParams]
  displacements = "disp_x disp_y"
[]

[Variables]
  [lambda]
    # block = 2
  []
[]

[AuxVariables]
  [disp_x]
    [InitialCondition]
      type = FunctionIC
      function = '0.01*sin(y*5)'
    []
  []
  [disp_y]
    [InitialCondition]
      type = FunctionIC
      function = '0.01*sin(x*5)'
    []
  []
[]

[BCs]
  [lambda]
    type = LagrangeInclinedNoDisplacementMultiplier
    variable = lambda
    boundary = outer
  []
[]

[Kernels]
  [diff]
    type = ADDiffusion
    variable = lambda
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
