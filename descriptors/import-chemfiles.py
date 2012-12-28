# import-chemfiles.py: imports molecules from the chem-file cml files
#
# see: http://chem-file.sourceforge.net/

import os
import sys
import json
import pymongo
import chemkit

# import a single molecule file into the collection
def importFile(fileName, collection):
  moleculeFile = chemkit.MoleculeFile(fileName)

  if not moleculeFile.read():
    print 'Error reading file: ' + moleculeFile.errorString()
    return False
  elif moleculeFile.isEmpty():
    print 'Error: file is empty'
    return False

  molecule = moleculeFile.molecule()

  # data values
  name = molecule.name()
  formula = molecule.formula()
  inchi = molecule.formula("inchi")
  inchikey = molecule.formula("inchikey")

  # atom counts
  atomCount = molecule.atomCount()
  hydrogenAtomCount = len([atom for atom in molecule.atoms() if atom.isTerminalHydrogen()])
  heavyAtomCount = atomCount - hydrogenAtomCount

  # descriptors
  mass = molecule.mass()

  molecularWeight = 0.0
  try: molecularWeight = float(molecule.data("Molecular weight"))
  except: pass

  monoisotopicWeight = 0.0
  try: monoisotopicWeight = float(molecule.data("Monoisotopic weight"))
  except: pass

  meltingPoint = 0.0
  try: meltingPoint = float(molecule.data("Melting point"))
  except: pass

  boilingPoint = 0.0
  try: boilingPoint = float(molecule.data("Boiling point"))
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
       {"molecular-weight" : molecularWeight,
        "monoisotopic-weight" : monoisotopicWeight,
        "melting-point" : meltingPoint,
        "boiling-point" : boilingPoint}
  }

  collection.update({"inchikey" : inchikey}, document, True)

  return True

# check arguments
if len(sys.argv) < 3:
  print 'Usage: ' + sys.argv[0] + ' [SETTINGS_FILE] [ROOT_DIRECTORY]'
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

# connect to mongochem mongo server
connection = pymongo.Connection(host, port)
db = connection[collection_name]
molecules_collection = db['molecules']

for (dirpath, dirnames, filenames) in os.walk(sys.argv[2]):
  for filename in [f for f in filenames if f.endswith(".cml")]:
    path = dirpath + '/' + filename
    print "loading: " + path[len(sys.argv[2]):]
    importFile(path, molecules_collection)
