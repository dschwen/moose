[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
  xmax = 2
  ymax = 2
  displacements = 'disp_x disp_y'
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
      function = x
    [../]
  [../]
[]

[Kernels]
  [./disp_x]
    type = Diffusion
    variable = disp_x
  [../]
  [./dt_disp_x]
    type = TimeDerivative
    variable = disp_x
  [../]
  [./disp_y]
    type = Diffusion
    variable = disp_y
  [../]
  [./dt_disp_y]
    type = TimeDerivative
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
  [./dt_u]
    type = TimeDerivative
    variable = u
    use_displaced_mesh = true
  [../]
[]

[BCs]
  [./disp_x_left]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  [../]
  [./disp_x_right]
    type = DirichletBC
    variable = disp_x
    boundary = right
    value = 1
  [../]
  [./u_left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  [../]
  [./u_right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 5
[]
