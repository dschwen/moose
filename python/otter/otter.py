#!/usr/bin/env python

import os
import sys
from FactorySystem.Factory import Factory
from FactorySystem.Parser import Parser
from FactorySystem.Warehouse import Warehouse
from FactorySystem.Parser import DupWalker
import hit
from mooseutils import HitNode, hit_parse

filename = 'test.plot'

factory = Factory()
warehouse = Warehouse()
parser = Parser(factory, warehouse)

pathname = os.path.dirname(os.path.realpath(sys.argv[0]))
pathname = os.path.abspath(pathname)
factory.loadPlugins([pathname], 'plugins', "IS_PLUGIN")

root_params = HitNode(hitnode=hit.parse('',''))
parser.parse(filename, root_params)

if parser.errors:
    for error in parser.errors:
        print error
    sys.exit(1)

active_objects = warehouse.getActiveObjects()
source_warehouse = {}

# initial setup
for obj in active_objects:
    if obj._system == 'DataSource':
        source_warehouse[obj._name] = obj
    if obj._system == 'PlotOutput':
        obj.registerDataSourceWarehouse(source_warehouse)

# execute
for obj in active_objects:
    if obj._system == 'PlotOutput':
        obj.execute()
