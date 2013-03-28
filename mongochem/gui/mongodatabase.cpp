/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include <set>

#include "mongodatabase.h"

#include <boost/range/algorithm.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <QSettings>

namespace MongoChem {

MongoDatabase::MongoDatabase()
{
  m_db = 0;
}

MongoDatabase::~MongoDatabase()
{
  disconnect();
}

MongoDatabase* MongoDatabase::instance()
{
  static MongoDatabase singleton;

  if (!singleton.isConnected()) {
    // Connect to the database.
    mongo::DBClientConnection *db = new mongo::DBClientConnection;

    QSettings settings;
    std::string host = settings.value("hostname").toString().toStdString();
    try {
      std::cout << "connecting to: " << host;
      std::flush(std::cout);
      db->connect(host);
      std::cout << " -- success" << std::endl;
    }
    catch (mongo::DBException &e) {
      std::cout << " -- failure" << std::endl;
      std::cerr << "Error: Failed to connect to MongoDB at '"
                << host
                << "': "
                << e.what()
                << std::endl;
      delete db;
      db = 0;
    }

    singleton.m_db = db;
  }

  return &singleton;
}

void MongoDatabase::disconnect()
{
  delete m_db;
  m_db = 0;
}

bool MongoDatabase::isConnected() const
{
  return m_db != 0;
}

mongo::DBClientConnection* MongoDatabase::connection() const
{
  return m_db;
}

std::auto_ptr<mongo::DBClientCursor>
MongoDatabase::query(const std::string &collection,
                     const mongo::Query &query_,
                     int limit,
                     int skip)
{
  if(!m_db)
    return std::auto_ptr<mongo::DBClientCursor>();

  return m_db->query(collection, query_, limit, skip);
}

std::auto_ptr<mongo::DBClientCursor>
MongoDatabase::queryMolecules(const mongo::Query &query_, int limit, int skip)
{
  return query(moleculesCollectionName(), query_, limit, skip);
}

std::string MongoDatabase::userName() const
{
  QSettings settings;
  return settings.value("user", "unknown").toString().toStdString();
}

MoleculeRef MongoDatabase::findMoleculeFromIdentifier(const std::string &identifier,
                                                      const std::string &format)
{
  if (!m_db)
    return MoleculeRef();

  std::string collection = moleculesCollectionName();
  mongo::BSONObj obj = m_db->findOne(collection, QUERY(format << identifier));
  return createMoleculeRefForBSONObj(obj);
}

MoleculeRef MongoDatabase::findMoleculeFromInChI(const std::string &inchi)
{
  return findMoleculeFromIdentifier(inchi, "inchi");
}

MoleculeRef MongoDatabase::findMoleculeFromInChIKey(const std::string &inchikey)
{
  return findMoleculeFromIdentifier(inchikey, "inchikey");
}

MoleculeRef MongoDatabase::findMoleculeFromBSONObj(const mongo::BSONObj *obj)
{
  if (!m_db)
    return MoleculeRef();

  mongo::BSONElement inchikeyElement = obj->getField("inchikey");
  if (inchikeyElement.eoo())
    return MoleculeRef();

  return findMoleculeFromInChIKey(inchikeyElement.str());
}

MoleculeRef
MongoDatabase::importMoleculeFromIdentifier(const std::string &identifier,
                                            const std::string &format)
{
  if (!m_db)
    return MoleculeRef();

  // create molecule
  boost::scoped_ptr<chemkit::Molecule>
    molecule(new chemkit::Molecule(identifier, format));

  if (!molecule) {
    // format is not supported or identifier is invalid
    return MoleculeRef();
  }

  // generate inchikey
  std::string inchikey = molecule->formula("inchikey");

  // check if molecule already exists
  MoleculeRef ref = findMoleculeFromInChIKey(inchikey);
  if (ref) {
    // molecule exists so just return a reference to it
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
  m_db->insert(moleculesCollectionName(), b.obj());

  return MoleculeRef(id);
}

mongo::BSONObj MongoDatabase::fetchMolecule(const MoleculeRef &molecule)
{
  if (!m_db)
    return mongo::BSONObj();

  std::string collection = moleculesCollectionName();
  return m_db->findOne(collection, QUERY("_id" << molecule.id()));
}

std::vector<mongo::BSONObj> MongoDatabase::fetchMolecules(const std::vector<MoleculeRef> &molecules)
{
  std::vector<mongo::BSONObj> objs;

  for (size_t i = 0; i < molecules.size(); ++i)
    objs.push_back(fetchMolecule(molecules[i]));

  return objs;
}

boost::shared_ptr<chemkit::Molecule> MongoDatabase::createMolecule(const MoleculeRef &ref)
{
  if (!ref.isValid())
    return boost::make_shared<chemkit::Molecule>();

  // fetch molecule object
  mongo::BSONObj obj = fetchMolecule(ref);

  // get inchi formula
  mongo::BSONElement inchiElement = obj.getField("inchi");
  if (inchiElement.eoo())
    return boost::make_shared<chemkit::Molecule>();

  std::string inchi = inchiElement.str();

  // create molecule from inchi
  chemkit::Molecule *molecule = new chemkit::Molecule(inchi, "inchi");

  // set molecule name
  mongo::BSONElement nameElement = obj.getField("name");
  if (!nameElement.eoo())
    molecule->setName(nameElement.str());

  return boost::shared_ptr<chemkit::Molecule>(molecule);
}

void MongoDatabase::addAnnotation(const MoleculeRef &ref,
                                  const std::string &comment)
{
  if (!ref.isValid())
    return;

  // add new annotation
  mongo::BSONObjBuilder annotation;
  annotation.append("user", userName());
  annotation.append("comment", comment);

  // store annotations
  m_db->update(moleculesCollectionName(),
               QUERY("_id" << ref.id()),
               BSON("$push" << BSON("annotations" << annotation.obj())),
               false,
               true);
}

void MongoDatabase::deleteAnnotation(const MoleculeRef &ref, size_t index)
{
  if (!ref.isValid())
    return;

  // identifer for the item in the annotations array
  std::stringstream id;
  id << "annotations." << index;

  // set the value at index to null
  m_db->update(moleculesCollectionName(),
               QUERY("_id" << ref.id()),
               BSON("$unset" << BSON(id.str() << 1)),
               false,
               true);

  // remove all null entries from the list
  mongo::BSONObjBuilder builder;
  builder.appendNull("annotations");
  m_db->update(moleculesCollectionName(),
               QUERY("_id" << ref.id()),
               BSON("$pull" << builder.obj()),
               false,
               true);
}

void MongoDatabase::updateAnnotation(const MoleculeRef &ref,
                                     size_t index,
                                     const std::string &comment)
{
  if (!ref.isValid())
    return;

  // identifer for the item in the annotations array
  std::stringstream id;
  id << "annotations." << index << ".comment";

  // update the record with the new comment
  m_db->update(moleculesCollectionName(),
               QUERY("_id" << ref.id()),
               BSON("$set" << BSON(id.str() << comment)),
               false,
               true);
}

void MongoDatabase::addTag(const MoleculeRef &ref, const std::string &tag)
{
  if (!ref.isValid())
    return;

  m_db->update(moleculesCollectionName(),
               QUERY("_id" << ref.id()),
               BSON("$addToSet" << BSON("tags" << tag)),
               false,
               true);
}

void MongoDatabase::removeTag(const MoleculeRef &ref, const std::string &tag)
{
  if (!ref.isValid())
    return;

  m_db->update(moleculesCollectionName(),
               QUERY("_id" << ref.id()),
               BSON("$pull" << BSON("tags" << tag)),
               false,
               true);
}

std::vector<std::string> MongoDatabase::fetchTags(const MoleculeRef &ref)
{
  std::vector<std::string> tags;

  mongo::BSONObj obj = fetchMolecule(ref);
  if (obj.hasField("tags") && obj["tags"].isABSONObj()) {
    try {
      boost::transform(obj["tags"].Array(),
                       std::back_inserter(tags),
                       boost::bind(&mongo::BSONElement::str, _1));
    }
    catch (...){
      // mongo threw a mongo::UserException or bson::assertion exception
      // which means the molecule does't have a tags array so just return
    }
  }

  return tags;
}

std::vector<std::string>
MongoDatabase::fetchTagsWithPrefix(const std::string &collection,
                                   const std::string &prefix,
                                   size_t limit)
{
  // get full collection name (e.g. "chem.molecules")
  QSettings settings;
  QString base_collection = settings.value("collection", "chem").toString();
  std::string collection_string = base_collection.toStdString()
                                  + "."
                                  + collection;

  // a limit of zero means we should return all tags
  if (limit == 0)
    limit = std::numeric_limits<size_t>::max();

  // fetch all documents with a tags array and return just the tags array
  mongo::BSONObj to_return = BSON("tags" << true);
  std::auto_ptr<mongo::DBClientCursor> cursor =
    m_db->query(collection_string,
                QUERY("tags" << BSON("$exists" << true)),
                0,
                0,
                &to_return);

  // build set of tags
  std::set<std::string> tags;

  // add each tag to the set
  while (cursor->more() && tags.size() < limit) {
    mongo::BSONObj obj = cursor->next();
    if (obj.isEmpty())
      continue;

    if (obj.hasField("tags") && obj["tags"].isABSONObj()) {
      try {
        std::vector<mongo::BSONElement> array = obj["tags"].Array();

        for (size_t i = 0; i < array.size() && tags.size() < limit; i++) {
          std::string tag = array[i].str();
          if (boost::starts_with(tag, prefix))
            tags.insert(tag);
        }
      }
      catch (...){
        // mongo threw a mongo::UserException or bson::assertion exception
        // which means the molecule does't have a tags array so just return
      }
    }
  }

  // return as a vector
  return std::vector<std::string>(tags.begin(), tags.end());
}

std::string MongoDatabase::moleculesCollectionName() const
{
  QSettings settings;
  QString collection = settings.value("collection", "chem").toString();
  return collection.toStdString() + ".molecules";
}

MoleculeRef MongoDatabase::createMoleculeRefForBSONObj(const mongo::BSONObj &obj) const
{
  if (obj.isEmpty())
    return MoleculeRef();

  mongo::BSONElement idElement;
  bool ok = obj.getObjectID(idElement);
  if (!ok)
    return MoleculeRef();

  return MoleculeRef(idElement.OID());
}

} // end MongoChem namespace
