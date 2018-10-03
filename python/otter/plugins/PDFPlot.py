#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from PlotOutput import PlotOutput
from FactorySystem import InputParameters
import matplotlib.pyplot as plt

class PDFPlot(PlotOutput):
    IS_PLUGIN = True

    def validParams():
        params = InputParameters()
        params.addRequiredParam('type', "The type of test of Tester to create for this test.")
        params.addRequiredParam('file', "The PDFPlot file write.")
        params.addRequiredParam('sources', 'DataSources objects to plot')
        return params
    validParams = staticmethod(validParams)

    def __init__(self, name, params):
        super(PDFPlot, self).__init__(name, params)
        self.specs = params

    # Called to launch the job
    def execute(self):
        plt.figure(figsize=(6.0, 4.0))

        for source in self.specs['sources'].split():
            data = self.getDataSource(source).getData()
            plt.plot(data[0], data[1], label=source)

        plt.legend()
        plt.savefig(self.specs['file'], bbox_inches='tight', pad_inches=0)
