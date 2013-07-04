/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "chemkit.h"

#include "mongodatabase.h"

#include <chemkit/molecule.h>

namespace MongoChem {

ChemKit::ChemKit()
{
}

boost::shared_ptr<chemkit::Molecule> ChemKit::createMolecule(
    const MoleculeRef &ref)
{
  if (!ref.isValid())
    return boost::make_shared<chemkit::Molecule>();

  // Fetch the molecule object from the database.
  MongoDatabase *db = MongoDatabase::instance();
  mongo::BSONObj obj = db->fetchMolecule(ref);

  // get inchi formula
  mongo::BSONElement inchiElement = obj.getField("inchi");
  if (inchiElement.eoo())
    return boost::make_shared<chemkit::Molecule>();

  std::string inchi = inchiElement.str();

  // Create a chemkit molecule from the InChI.
  chemkit::Molecule *molecule = new chemkit::Molecule(inchi, "inchi");
  mongo::BSONElement nameElement = obj.getField("name");
  if (!nameElement.eoo())
    molecule->setName(nameElement.str());

  return boost::shared_ptr<chemkit::Molecule>(molecule);
}

MoleculeRef ChemKit::importMoleculeFromIdentifier(const std::string &identifier,
                                                  const std::string &format)
{
  MongoDatabase *db = MongoDatabase::instance();
  if (!db || !db->connection())
    return MoleculeRef();

  // create molecule
  boost::scoped_ptr<chemkit::Molecule>
    molecule(new chemkit::Molecule(identifier, format));

  if (!molecule)
    return MoleculeRef();

  // Generate an InChI key for the molecule.
  std::string inchikey = molecule->formula("inchikey");

  // Check if molecule already exists in the database
  MoleculeRef ref = db->findMoleculeFromInChIKey(inchikey);
  if (ref) {
    // The molecule exists, so just return a reference to it.
    return ref;
  }

  // generate identifiers
  std::string formula = molecule->formula();
  std::string inchi = molecule->formula("inchi");
  std::string smiles = molecule->formula("smiles");

  // generate descriptors
  double mass = molecule->mass();
  int atomCount = static_cast<int>(molecule->atomCount());
  int heavyAtomCount =
    static_cast<int>(molecule->atomCount() - molecule->atomCount("H"));

  // create object id
  mongo::OID id = mongo::OID::gen();

  // create bson object
  mongo::BSONObjBuilder b;
  b << "_id" << id;
  b << "formula" << formula;
  b << "inchi" << inchi;
  b << "inchikey" << inchikey;
  b << "smiles" << smiles;
  b << "mass" << mass;
  b << "atomCount" << atomCount;
  b << "heavyAtomCount" << heavyAtomCount;

  // insert molecule
  db->connection()->insert(db->moleculesCollectionName(), b.obj());

  return MoleculeRef(id);
}

}
