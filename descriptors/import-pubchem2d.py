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
  # data values
  name = molecule.data("PUBCHEM_IUPAC_TRADITIONAL_NAME")
  if not len(name):
    name = molecule.data("PUBCHEM_IUPAC_SYSTEMATIC_NAME")

  formula = molecule.formula()
  inchi = molecule.formula("inchi")
  inchikey = molecule.formula("inchikey")

  # atom counts
  atomCount = molecule.atomCount()
  hydrogenAtomCount = len([atom for atom in molecule.atoms() if atom.isTerminalHydrogen()])
  heavyAtomCount = atomCount - hydrogenAtomCount

  # descriptors
  mass = 0.0
  try: mass = float(molecule.data("PUBCHEM_MOLECULAR_WEIGHT"))
  except: pass

  tpsa = 0.0
  try: tpsa = float(molecule.data("PUBCHEM_CACTVS_TPSA"))
  except: pass

  xlogp3 = 0.0
  try: xlogp3 = float(molecule.data("PUBCHEM_XLOGP3_AA"))
  except: pass

  vabc = 0.0
  try: vabc = float(molecule.descriptor("vabc"))
  except: pass

  rotatable_bonds = 0.0
  try: rotatable_bonds = int(molecule.descriptor("rotatable-bonds"))
  except: pass

  # insert molecule document
  document = {
     "name" : name,
     "formula" : formula,
     "inchi" : inchi,
     "inchikey" : inchikey,
     "atomCount" : atomCount,
     "heavyAtomCount" : heavyAtomCount,
     "mass" : mass,
     "descriptors" :
       {"mass" : mass,
        "tpsa" : tpsa,
        "xlogp3" : xlogp3,
        "vabc" : vabc,
        "rotatable-bonds" : rotatable_bonds}
  }

  molecules_collection.update({"inchikey" : inchikey}, document, True)
