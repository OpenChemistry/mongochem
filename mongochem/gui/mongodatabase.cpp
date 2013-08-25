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

#include "mongodatabase.h"

#include <boost/range/algorithm.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <QtCore/QSettings>

#include <set>

namespace MongoChem {

using std::string;
using std::stringstream;
using std::cerr;
using std::cout;
using std::endl;
using std::flush;
using std::vector;

MongoDatabase::MongoDatabase() : m_db(NULL)
{
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
    string host = settings.value("hostname").toString().toStdString();
    try {
      cout << "connecting to: " << host;
      flush(cout);
      db->connect(host);
      cout << " -- success" << endl;
    }
    catch (mongo::DBException &e) {
      cout << " -- failure" << endl;
      cerr << "Error: Failed to connect to MongoDB at '"
           << host
           << "': "
           << e.what()
           << endl;
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
MongoDatabase::query(const string &collection,
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

string MongoDatabase::userName() const
{
  QSettings settings;
  return settings.value("user", "unknown").toString().toStdString();
}

MoleculeRef MongoDatabase::findMoleculeFromIdentifier(const string &identifier,
                                                      const string &format)
{
  if (!m_db)
    return MoleculeRef();

  string collection = moleculesCollectionName();
  mongo::BSONObj obj = m_db->findOne(collection, QUERY(format << identifier));
  return createMoleculeRefForBSONObj(obj);
}

MoleculeRef MongoDatabase::findMoleculeFromInChI(const string &inchi)
{
  return findMoleculeFromIdentifier(inchi, "inchi");
}

MoleculeRef MongoDatabase::findMoleculeFromInChIKey(const string &inchikey)
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

mongo::BSONObj MongoDatabase::fetchMolecule(const MoleculeRef &molecule)
{
  if (!m_db)
    return mongo::BSONObj();

  string collection = moleculesCollectionName();
  return m_db->findOne(collection, QUERY("_id" << mongo::OID(molecule.id())));
}

vector<mongo::BSONObj> MongoDatabase::fetchMolecules(const vector<MoleculeRef> &molecules)
{
  vector<mongo::BSONObj> objs;

  for (size_t i = 0; i < molecules.size(); ++i)
    objs.push_back(fetchMolecule(molecules[i]));

  return objs;
}

void MongoDatabase::addAnnotation(const MoleculeRef &ref,
                                  const string &comment)
{
  if (!ref.isValid())
    return;

  // add new annotation
  mongo::BSONObjBuilder annotation;
  annotation.append("user", userName());
  annotation.append("comment", comment);

  // store annotations
  m_db->update(moleculesCollectionName(),
               QUERY("_id" << mongo::OID(ref.id())),
               BSON("$push" << BSON("annotations" << annotation.obj())),
               false,
               true);
}

void MongoDatabase::deleteAnnotation(const MoleculeRef &ref, size_t index)
{
  if (!ref.isValid())
    return;

  // identifer for the item in the annotations array
  stringstream id;
  id << "annotations." << index;

  // set the value at index to null
  m_db->update(moleculesCollectionName(),
               QUERY("_id" << mongo::OID(ref.id())),
               BSON("$unset" << BSON(id.str() << 1)),
               false,
               true);

  // remove all null entries from the list
  mongo::BSONObjBuilder builder;
  builder.appendNull("annotations");
  m_db->update(moleculesCollectionName(),
               QUERY("_id" << mongo::OID(ref.id())),
               BSON("$pull" << builder.obj()),
               false,
               true);
}

void MongoDatabase::updateAnnotation(const MoleculeRef &ref,
                                     size_t index,
                                     const string &comment)
{
  if (!ref.isValid())
    return;

  // identifer for the item in the annotations array
  stringstream id;
  id << "annotations." << index << ".comment";

  // update the record with the new comment
  m_db->update(moleculesCollectionName(),
               QUERY("_id" << mongo::OID(ref.id())),
               BSON("$set" << BSON(id.str() << comment)),
               false,
               true);
}

void MongoDatabase::addTag(const MoleculeRef &ref, const string &tag)
{
  if (!ref.isValid())
    return;

  m_db->update(moleculesCollectionName(),
               QUERY("_id" << mongo::OID(ref.id())),
               BSON("$addToSet" << BSON("tags" << tag)),
               false,
               true);
}

void MongoDatabase::removeTag(const MoleculeRef &ref, const string &tag)
{
  if (!ref.isValid())
    return;

  m_db->update(moleculesCollectionName(),
               QUERY("_id" << mongo::OID(ref.id())),
               BSON("$pull" << BSON("tags" << tag)),
               false,
               true);
}

vector<string> MongoDatabase::fetchTags(const MoleculeRef &ref)
{
  vector<string> tags;

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

vector<string> MongoDatabase::fetchTagsWithPrefix(const string &collection,
                                                  const string &prefix,
                                                  size_t limit)
{
  // get full collection name (e.g. "chem.molecules")
  QSettings settings;
  QString base_collection = settings.value("collection", "chem").toString();
  string collection_string = base_collection.toStdString()
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
  std::set<string> tags;

  // add each tag to the set
  while (cursor->more() && tags.size() < limit) {
    mongo::BSONObj obj = cursor->next();
    if (obj.isEmpty())
      continue;

    if (obj.hasField("tags") && obj["tags"].isABSONObj()) {
      try {
        vector<mongo::BSONElement> array = obj["tags"].Array();

        for (size_t i = 0; i < array.size() && tags.size() < limit; i++) {
          string tag = array[i].str();
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
  return vector<string>(tags.begin(), tags.end());
}

string MongoDatabase::moleculesCollectionName() const
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

  return MoleculeRef(idElement.OID().str());
}

} // end MongoChem namespace
