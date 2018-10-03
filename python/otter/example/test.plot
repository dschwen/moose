[Outputs]
  [./pdf1]
    type = PDFPlot
    sources = 'fgr1 fgr2'
    file = pdf1.pdf
  [../]
[]

[DataSources]
  [./fgr1]
    type = CSV
    file = out1.csv
    x_data = time
    y_data = fgr
  [../]
  [./fgr2]
    type = CSV
    file = out2.csv
    x_data = time
    y_data = release
  [../]
[]
