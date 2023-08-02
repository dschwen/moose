[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 1
  []
  [block]
    type = SubdomainBoundingBoxGenerator
    input = gen
    top_right = '1 1 0'
    bottom_left = '0.5 0 0'
    block_id = 1
  []
[]

[Problem]
  solve = false
[]

[Materials]
  # material restricted to block 1
  [prop]
    type = GenericFunctionMaterial
    prop_names = prop
    prop_values = 1
    block = 1
  []

  # to have material properties on every block
  [dummy]
    type = GenericConstantMaterial
    prop_names = dummy
    prop_values = 0
  []
[]

[AuxVariables]
  [u]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  # request property as optional on block 0 where it does not exist
  # expect = false, so this should pass
  [optional]
    type = OptionalTestAux
    variable = u
    prop = prop
    adprop = adprop
    expect = false
    adexpect = false
    block = 0
  []
[]

[Executioner]
  type = Steady
[]
