# make-descriptors-csv-file.py: creates a csv file with descriptor values

import os
import sys
import chemkit

if len(sys.argv) < 4:
    print 'usage: ' + sys.argv[0] + ' [INPUT_FILENAME] [OUTPUT_FILENAME] [DESCRIPTORS...]'

input_filename = sys.argv[1]
output_filename = sys.argv[2]

# read input file
input_file = chemkit.MoleculeFile(input_filename)
if not input_file.read():
    print 'error reading file: ' + input_file.errorString()
    sys.exit(-1)

# create output file
output_file = open(output_filename, "w")

# get descriptor names
descriptors = sys.argv[3:]

# write header
output_file.write("inchikey,")
for descriptor in descriptors:
    output_file.write(descriptor + ",")
output_file.write("\n")

for molecule in input_file.molecules():
    output_file.write(molecule.formula("inchikey") + ",")

    for descriptor in descriptors:
        output_file.write(str(molecule.descriptor(descriptor)) + ",")

    output_file.write("\n")
