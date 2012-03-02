# import-pubchem2d.py: imports molecules from 2D pubchem files

import os
import sys
import json
import pymongo
import chemkit

# check arguments
if len(sys.argv) < 3:
  print 'Usage: ' + sys.argv[0] + ' [SETTINGS_FILE] [DATA_FILE]'
  sys.exit(-1)

# connection settings
try:
  settings = json.load(file(sys.argv[1]))
  host = settings['server']['host']
  port = settings['server']['port']
  collection_name = settings['server']['collection']
except:
  print 'Failed to load server settings'
  sys.exit(-1)

# connect to chemdata mongo server
connection = pymongo.Connection(host, port)
db = connection[collection_name]
molecules_collection = db['molecules']

# open and read pubchem file
fileName = sys.argv[2]
moleculeFile = chemkit.MoleculeFile(fileName)
if not moleculeFile.read():
  print 'Error reading file: ' + moleculeFile.errorString()
  sys.exit(-1)

print 'Read file \'' + os.path.basename(fileName) + '\' (%d molecules)' % moleculeFile.moleculeCount()

for molecule in moleculeFile.molecules():
  inchikey = molecule.formula("inchikey")

  elements = []
  coordinates = []
  for atom in molecule.atoms():
    elements.append(atom.atomicNumber())
    coordinates.append(atom.x())
    coordinates.append(atom.y())
    coordinates.append(atom.z())

  bonds = []
  bond_orders = []
  for bond in molecule.bonds():
    bonds.append(bond.atom1().index())
    bonds.append(bond.atom2().index())
    bond_orders.append(bond.order())

  document = {
    "atoms" : {
      "elements" : elements,
      "coords" : {
        "3d" : coordinates
      }
    },
    "bonds" : {
      "connections" : bonds,
      "orders" : bond_orders
    }
  }

  molecules_collection.update({"inchikey" : inchikey}, {"$set" : document}, False)
