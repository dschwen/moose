[Mesh]
  type = GeneratedMesh
  dim = 2
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./u]
  [../]
  [./disp_x]
    [./InitialCondition]
      type = FunctionIC
      function = '0'#^2 + y'
    [../]
  [../]
  [./disp_y]
    [./InitialCondition]
      type = FunctionIC
      function = '0'#x + sin(y)'
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
    type = DisplacedBodyForce
    variable = u
    function = 1
    displacements = 'disp_x disp_y'
    use_displaced_mesh = true
  [../]
  # [./u]
  #   type = BodyForce
  #   variable = u
  #   function = 1
  #   use_displaced_mesh = true
  # [../]
  # [./u]
  #   type = DisplacedDiffusion
  #   variable = u
  #   displacements = 'disp_x disp_y'
  #   use_displaced_mesh = true
  # [../]
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
