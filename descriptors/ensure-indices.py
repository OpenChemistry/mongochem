# ensure-indices.py: ensures the proper indices for chemdata

import sys
import json
import pymongo

# check arguments
if len(sys.argv) < 2:
  print 'Usage: ' + sys.argv[0] + ' [SETTINGS_FILE]'
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

# ensure indices
molecules_collection.ensure_index("inchikey", True)
molecules_collection.ensure_index("heavyAtomCount", False)
