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

#include <chemkit/fingerprint.h>
#include <chemkit/molecule.h>

namespace MongoChem {

using std::string;
using std::vector;

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

  string inchi = inchiElement.str();

  // Create a chemkit molecule from the InChI.
  chemkit::Molecule *molecule = new chemkit::Molecule(inchi, "inchi");
  mongo::BSONElement nameElement = obj.getField("name");
  if (!nameElement.eoo())
    molecule->setName(nameElement.str());

  return boost::shared_ptr<chemkit::Molecule>(molecule);
}

boost::shared_ptr<chemkit::Molecule> ChemKit::createMolecule(const string &identifier,
                                                             const string &format)
{
  chemkit::Molecule *molecule = new chemkit::Molecule(identifier, format);
  return boost::shared_ptr<chemkit::Molecule>(molecule);
}

MoleculeRef ChemKit::importMoleculeFromIdentifier(const string &identifier,
                                                  const string &format)
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
  string inchikey = molecule->formula("inchikey");

  // Check if molecule already exists in the database
  MoleculeRef ref = db->findMoleculeFromInChIKey(inchikey);
  if (ref) {
    // The molecule exists, so just return a reference to it.
    return ref;
  }

  // generate identifiers
  string formula = molecule->formula();
  string inchi = molecule->formula("inchi");
  string smiles = molecule->formula("smiles");

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

  return MoleculeRef(id.str());
}

vector<MoleculeRef> ChemKit::similarMolecules(const MoleculeRef &ref,
                                              const vector<MoleculeRef> &refs,
                                              size_t count)
{
  return similarMolecules(createMolecule(ref), refs, count);
}

vector<MoleculeRef> ChemKit::similarMolecules(const boost::shared_ptr<chemkit::Molecule> &molecule,
                                              const vector<MoleculeRef> &refs,
                                              size_t count)
{
  vector<MoleculeRef> molecules;
  if (!molecule)
    return molecules;

  // Create an fp2 fingerprint for the molecule given.
  boost::scoped_ptr<chemkit::Fingerprint>
      fp2(chemkit::Fingerprint::create("fp2"));
  if (!fp2)
    return molecules;

  // Calculate the fingerprint for the input molecule.
  chemkit::Bitset fingerprint = fp2->value(molecule.get());

  // Get access to the database.
  MongoDatabase *db = MongoDatabase::instance();

  // Calculate the tanimoto similarity value for each molecule.
  std::map<float, MoleculeRef> sorted;
  for (size_t i = 0; i < refs.size(); ++i) {
    float similarity = 0;
    mongo::BSONObj obj = db->fetchMolecule(refs[i]);
    mongo::BSONElement element = obj.getField("fp2_fingerprint");
    if (element.ok()) {
      // There is already a fingerprint stored for the molecule so load and use
      // that for calculating the similarity value.
      int len = 0;
      const char *binData = element.binData(len);
      vector<size_t> binDataBlockVector(fingerprint.num_blocks());

      memcpy(&binDataBlockVector[0],
             binData,
             binDataBlockVector.size() * sizeof(size_t));

      chemkit::Bitset fingerprintValue(binDataBlockVector.begin(),
                                       binDataBlockVector.end());

      // Ensure that the fingerprints are the same size. This is necessary
      // because different platforms may have different padding for the bits.
      fingerprintValue.resize(fingerprint.size());

      similarity =
        static_cast<float>(
          chemkit::Fingerprint::tanimotoCoefficient(fingerprint,
                                                    fingerprintValue));
    }
    else {
      // There is not a fingerprint calculated for the molecule so create the
      // molecule and calculate the fingerprint directly.
      boost::shared_ptr<chemkit::Molecule> otherMolecule = createMolecule(refs[i]);

      if (otherMolecule) {
        similarity =
          static_cast<float>(
            chemkit::Fingerprint::tanimotoCoefficient(fingerprint,
                                                      fp2->value(
                                                        otherMolecule.get())));
      }
      else {
        similarity = 0.0f;
      }
    }
    sorted[similarity] = refs[i];
  }

  // Clamp number of output molecules to the number of input molecules.
  count = std::min(count, refs.size());

  // Build a vector of refs composed of the most similar molecules.
  molecules.resize(count);

  std::map<float, MoleculeRef>::const_reverse_iterator iter = sorted.rbegin();
  for (size_t i = 0; i < count; ++i)
    molecules[i] = iter++->second;

  return molecules;
}

int ChemKit::heavyAtomCount(const string &identifier, const string &format)
{
  chemkit::Molecule molecule(identifier, format);
  return static_cast<int>(molecule.atomCount() - molecule.atomCount("H"));
}

}
