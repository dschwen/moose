#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

from DataSource import DataSource
from FactorySystem import InputParameters
# import os, sys, shutil
import mooseutils

class CSV(DataSource):
    IS_PLUGIN = True

    def validParams():
        params = InputParameters()
        params.addRequiredParam('type', "The type of test of Tester to create for this test.")
        params.addRequiredParam('file', "The CSV file to read.")
        params.addRequiredParam('x_data', 'X data CSV column name')
        params.addRequiredParam('y_data', 'Y data CSV column name')
        return params
    validParams = staticmethod(validParams)

    def __init__(self, name, params):
        super(CSV, self).__init__(name, params)
        self.specs = params
        # Read data from file
        data = mooseutils.PostprocessorReader(params['file'])
        self._data = (data[params['x_data']].tolist(), data[params['y_data']].tolist())

    # Called to launch the job
    def getData(self):
        return self._data
