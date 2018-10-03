#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

class PlotOutput(object):
    def __init__(self, name, params):
        self._name = name
        self._system = "PlotOutput"

    # register all data source objects here
    def registerDataSourceWarehouse(self, source_warehouse):
        self._source_warehouse = source_warehouse

    # Called to launch the job
    def execute(self):
        raise Exception("override this!")

    # get datasource by name
    def getDataSource(self, name):
        return self._source_warehouse[name]
