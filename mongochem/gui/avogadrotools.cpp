/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2013 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "avogadrotools.h"

#include "cjsonexporter.h"
#include "moleculeref.h"
#include "mongodatabase.h"

#include <mongo/client/dbclient.h>

#include <avogadro/io/cjsonformat.h>

#include <avogadro/core/molecule.h>

using Avogadro::Io::CjsonFormat;

namespace MongoChem {

AvogadroTools::AvogadroTools()
{
}

bool AvogadroTools::createMolecule(const MoleculeRef &mcMol,
                                   Avogadro::Core::Molecule &avoMol)
{
  MongoDatabase *db = MongoDatabase::instance();
  if (!db)
    return false;

  mongo::BSONObj obj = db->fetchMolecule(mcMol);
  if (!obj.hasField("3dStructure"))
    return false;

  std::string cjson = CjsonExporter::toCjson(obj);
  CjsonFormat reader;
  return reader.readString(cjson, avoMol);
}

} // namespace MongoChem
