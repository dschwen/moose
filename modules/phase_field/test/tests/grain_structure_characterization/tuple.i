[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
[]

[GlobalParams]
  op_num = 10
  var_name_base = gr
  grain_num = 25
[]

[Variables]
  [./PolycrystalVariables]
  [../]
[]

[AuxVariables]
  [./S1]
  [../]
  [./S2]
  [../]
  [./S3]
  [../]
  [./S4]
  [../]
[]

[AuxKernels]
  [./S1]
    type = TupleProductSum
    variable = S1
    v = 'gr0 gr1 gr2 gr3 gr4 gr5 gr6 gr7 gr8 gr9'
    k = 1
  [../]
  [./S2]
    type = TupleProductSum
    variable = S2
    v = 'gr0 gr1 gr2 gr3 gr4 gr5 gr6 gr7 gr8 gr9'
    k = 2
  [../]
  [./S3]
    type = TupleProductSum
    variable = S3
    v = 'gr0 gr1 gr2 gr3 gr4 gr5 gr6 gr7 gr8 gr9'
    k = 3
  [../]
  [./S4]
    type = TupleProductSum
    variable = S4
    v = 'gr0 gr1 gr2 gr3 gr4 gr5 gr6 gr7 gr8 gr9'
    k = 4
  [../]
[]

[UserObjects]
  [./grain_tracker]
    type = GrainTracker
  [../]
  [./voronoi]
    type = PolycrystalVoronoi
    int_width = 0.05
  [../]
[]

[ICs]
  [./PolycrystalICs]
    [./PolycrystalColoringIC]
      polycrystal_ic_uo = voronoi
    [../]
  [../]
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  hide = 'gr0 gr1 gr2 gr3 gr4 gr5 gr6 gr7 gr8 gr9'
[]
