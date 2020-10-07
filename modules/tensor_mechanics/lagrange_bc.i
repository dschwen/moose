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
  [lowerd]
    type = LowerDBlockFromSidesetGenerator
    input = boundary
    new_block_id = 2
    sidesets = outer
  []
[]

[GlobalParams]
  displacements = "disp_x disp_y"
[]

[Variables]
  [lambda]
    family = MONOMIAL
    order = CONSTANT
    block = 2
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

[Kernels]
  [lambda]
    type = LagrangeInclinedNoDisplacement
    variable = lambda
    block = 2
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
