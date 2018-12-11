[Mesh]
  type = GeneratedMesh
  dim = 2
  displacements = 'disp_x disp_y'
  nx = 2
  ny = 2
  xmax = 1
[]

[Variables]
  [./disp_x]
    [./InitialCondition]
      type = FunctionIC
      function = 'x^2 + y'
    [../]
  [../]
  [./disp_y]
    [./InitialCondition]
      type = FunctionIC
      function = 'x + sin(y)'
    [../]
  [../]
  [./u]
    [./InitialCondition]
      type = FunctionIC
      function = x^3+y
    [../]
  [../]
[]

[Kernels]
  [./disp_x]
    type = Diffusion
    variable = disp_x
  [../]
  [./disp_y]
    type = Diffusion
    variable = disp_y
  [../]
  [./u]
    type = DisplacedDiffusion
    variable = u
    displacements = 'disp_x disp_y'
    use_displaced_mesh = true
  [../]
  # [./u]
  #   type = Diffusion
  #   variable = u
  #   use_displaced_mesh = true
  # [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
[]
