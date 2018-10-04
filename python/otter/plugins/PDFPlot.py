#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from OtterOutput import OtterOutput
from FactorySystem import InputParameters
import matplotlib.pyplot as plt

class PDFPlot(OtterOutput):
    IS_PLUGIN = True

    def validParams():
        params = OtterOutput.validParams()
        params.addRequiredParam('file', "The PDFPlot file write.")
        params.addRequiredParam('sources', 'DataSources objects to plot')
        return params
    validParams = staticmethod(validParams)

    def __init__(self, name, params):
        super(PDFPlot, self).__init__(name, params)

    # Called to launch the job
    def execute(self):
        plt.figure(figsize=(6.0, 4.0))
        params = self._specs

        for source in params['sources'].split():
            data = self.getDataFrom(source)
            plt.plot(data[0], data[1], label=source)

        plt.legend()
        plt.savefig(params['file'], bbox_inches='tight', pad_inches=0)
