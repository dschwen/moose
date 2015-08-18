[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Variables]
  [./a]
    [./InitialCondition]
      type = RandomIC
    [../]
  [../]
  [./b]
  [../]
[]

[Kernels]
  [./a_res]
    type = Diffusion
    variable = a
  [../]
  [./a_dt]
    type = TimeDerivative
    variable = a
  [../]
  [./b_res]
    type = Diffusion
    variable = b
  [../]
  [./b_dt]
    type = TimeDerivative
    variable = b
  [../]
[]

[BCs]
  [./Periodic]
    [./auto]
      auto_direction = 'x y'
      variable = 'a'
    [../]
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
[]
