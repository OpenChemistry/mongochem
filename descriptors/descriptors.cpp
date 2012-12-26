/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2011 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include <fstream>
#include <iostream>
#include <string>

#include <openbabel/obconversion.h>
#include <openbabel/mol.h>

int main(int argc, char *argv[])
{
  if (argc != 2) {
    // Exit - we expect the name of an input file and output to the standard out
    std::cerr << "Error: expect one argument - path of input file." << std::endl;
    return 1;
  }

  OpenBabel::OBFormat *inFormat = NULL;
  OpenBabel::OBConversion conv(&std::cin, &std::cout);
  OpenBabel::OBMol mol;
  inFormat = conv.FormatFromExt(argv[1]);
  conv.SetInFormat(inFormat);
  std::ifstream in;
  in.open(argv[1]);
  conv.Read(&mol, &in);
  in.close();

  // Write out a few parameters.
  std::cout << "[Formula]\n" << mol.GetSpacedFormula() << std::endl;
  std::cout << "[Molecular weight]\n" << mol.GetMolWt() << std::endl;

  // Write out our file formats.
  std::cout << "[smiles]\n";
  conv.SetOutFormat("smi");
  conv.Write(&mol);
  std::cout << "[canonical smiles]\n";
  conv.SetOutFormat("can");
  conv.Write(&mol);
  std::cout << "[inchi]\n";
  conv.SetOutFormat("inchi");
  conv.Write(&mol);
  std::cout << "[inchikey]\n";
  conv.SetOptions("K", conv.OUTOPTIONS);
  conv.Write(&mol);
  std::cout << "[XYZ]\n";
  conv.SetOutFormat("xyz");
  conv.Write(&mol);
  std::cout << "[end]\n";
  std::cout << "[CML]\n";
  conv.SetOutFormat("cml");
  conv.Write(&mol);
  std::cout << "[end]\n";
  //std::cout << "[SVG]\n";
  //conv.SetOutFormat("svg");
  //conv.Write(&mol);
  //std::cout << "[end]\n";

  // Let them know we are finished, should be done after all output is complete.
  std::cout << "[complete]" << std::endl;

  return 0;
}
