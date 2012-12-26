# import-obabel-diagrams.py: add diagrams for molecules from openbabel

import sys
import json
import rsvg
import cairo
import pybel
import pymongo
from pymongo.binary import Binary, BINARY_SUBTYPE
from cStringIO import StringIO

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

# connect to mongochem mongo server
connection = pymongo.Connection(host, port)
db = connection[collection_name]
molecules_collection = db['molecules']

for document in molecules_collection.find():
  try:
    inchi = document['inchi']
    inchikey = document['inchikey']
  except:
    continue

  # build molecule from inchi
  try:
    molecule = pybel.readstring("inchi", str(inchi))
    if not len(molecule.atoms):
      continue
  except:
    continue

  # generate svg
  svg = molecule.write("svg")
  if not len(svg):
    continue

  # load svg data
  handle = rsvg.Handle(None, svg)

  # setup image
  image = cairo.ImageSurface(cairo.FORMAT_ARGB32, 250, 250)
  ctx = cairo.Context(image)
  ctx.fill()

  # scale to 250x250
  scale = 250.0 / max(handle.get_property("height"),
                      handle.get_property("width"))
  ctx.scale(scale, scale)

  # translate to center
  height = scale * handle.get_property("height")
  width = scale * handle.get_property("width")
  ctx.translate(125.0 - (0.5 * width),
                125.0 - (0.5 * height))

  # render image
  handle.render_cairo(ctx)

  # write png data
  png = StringIO()
  image.write_to_png(png)

  # insert diagram into document
  molecules_collection.update({"inchikey" : str(inchikey)},
                              {"$set" : {"diagram" : Binary(png.getvalue(), BINARY_SUBTYPE)}},
                              False)
