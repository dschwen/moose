# Difference between the fission gas curves
[./diff]
  type = Difference
  source1 = fgr1
  source2 = fgr2
[../]

[./pdf1]
  type = PDFPlot
  sources = 'fgr1 fgr2'
  file = fgr.pdf
[../]
[./pdf2]
  type = PDFPlot
  sources = 'diff'
  file = diff.pdf
[../]

# Fission gas curves
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
