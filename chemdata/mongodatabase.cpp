/******************************************************************************

  This source file is part of the ChemData project.

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

// === MongoDatabase ======================================================= //
/// \class MongoDatabase
/// \brief The MongoDatabase class represents a connection to a
///        Mongo database.
///
/// The MongoDatabase class is implemented as a singleton. The static
/// instance() method is used to retrieve a handle to the database.
///
/// The find*() methods in this class allow users to query for molecules
/// using various identifiers and return MoleculeRef objects representing
/// the molecules found.
///
/// The fetch*() methods take MoleculeRef's and return BSONObj's containing
/// the corresponding molecular data.
///
/// \warning The first invocation of \p instance() forms a persistant
///          connection to the mongo database. This method is not reentrant
///          and should be called only from a single thread.

// --- Construction and Destruction ---------------------------------------- //
/// Creates a new mongo database object. This constructor should not
/// be called by users; rather the instance() method should be used
/// to retrieve a handle to the mongo database.
MongoDatabase::MongoDatabase()
{
  m_db = 0;
}

/// Destroys the mongo database object.
MongoDatabase::~MongoDatabase()
{
  delete m_db;
}

/// Returns an instance of the singleton mongo database.
MongoDatabase* MongoDatabase::instance()
{
  static MongoDatabase singleton;

  if (!singleton.isConnected()) {
    // connect to database
    mongo::DBClientConnection *db = new mongo::DBClientConnection;

    QSettings settings;
    std::string host = settings.value("hostname").toString().toStdString();
    try {
      db->connect(host);
      std::cout << "Connected to: " << host << std::endl;
      singleton.m_db = db;
    }
    catch (mongo::DBException &e) {
      std::cerr << "Failed to connect to MongoDB at '"
                << host
                << "': "
                << e.what()
                << std::endl;
      delete db;
    }
  }

  return &singleton;
}

/// Returns \c true if the database object is connected to the mongo
/// database server.
bool MongoDatabase::isConnected() const
{
  return m_db != 0;
}

/// Returns the connection to the mongo database.
mongo::DBClientConnection* MongoDatabase::connection() const
{
  return m_db;
}

// --- Querying ------------------------------------------------------------ //
/// Queries the database for a molecule molecule with \p identifier in
/// \p format.
MoleculeRef MongoDatabase::findMoleculeFromIdentifer(const std::string &identifier,
                                                     const std::string &format)
{
  if (!m_db)
    return MoleculeRef();

  std::string collection = moleculesCollectionName();

  mongo::BSONObj obj = m_db->findOne(collection, QUERY(format << identifier));

  return createMoleculeRefForBSONObj(obj);
}

/// Returns a molecule ref corresponding to the molecule with \p inchi.
MoleculeRef MongoDatabase::findMoleculeFromInChI(const std::string &inchi)
{
  return findMoleculeFromIdentifer(inchi, "inchi");
}

/// Returns a molecule ref corresponding to the molecule with \p inchikey.
MoleculeRef MongoDatabase::findMoleculeFromInChIKey(const std::string &inchikey)
{
  return findMoleculeFromIdentifer(inchikey, "inchikey");
}

/// Returns a molecule ref corresponding to the molecule represented
/// by \p obj.
MoleculeRef MongoDatabase::findMoleculeFromBSONObj(const mongo::BSONObj *obj)
{
  if (!m_db)
    return MoleculeRef();

  mongo::BSONElement inchikeyElement = obj->getField("inchikey");
  if (inchikeyElement.eoo())
    return MoleculeRef();

  return findMoleculeFromInChIKey(inchikeyElement.str());
}

// --- Fetching ------------------------------------------------------------ //
/// Returns a BSONObj containing the data for the molecule referenced
/// by \p molecule.
mongo::BSONObj MongoDatabase::fetchMolecule(const MoleculeRef &molecule)
{
  if (!m_db)
    return mongo::BSONObj();

  std::string collection = moleculesCollectionName();

  return m_db->findOne(collection, QUERY("_id" << molecule.id()));
}

/// Returns a vector of BSONObj's containing the data for the molecules
/// referenced by \p molecules.
std::vector<mongo::BSONObj> MongoDatabase::fetchMolecules(const std::vector<MoleculeRef> &molecules)
{
  std::vector<mongo::BSONObj> objs;

  for(size_t i = 0; i < molecules.size(); i++){
    objs.push_back(fetchMolecule(molecules[i]));
  }

  return objs;
}

/// Creates a new molecule object for \p ref. The ownership of the returned
/// molecule object is passed to the caller.
boost::shared_ptr<chemkit::Molecule> MongoDatabase::createMolecule(const MoleculeRef &ref)
{
  if (!ref.isValid())
    return boost::shared_ptr<chemkit::Molecule>();

  // fetch molecule object
  mongo::BSONObj obj = fetchMolecule(ref);

  // get inchi formula
  mongo::BSONElement inchiElement = obj.getField("inchi");
  if (inchiElement.eoo())
    return boost::shared_ptr<chemkit::Molecule>();

  std::string inchi = inchiElement.str();

  // create molecule from inchi
  chemkit::Molecule *molecule = new chemkit::Molecule(inchi, "inchi");

  // set molecule name
  mongo::BSONElement nameElement = obj.getField("name");
  if (!nameElement.eoo())
    molecule->setName(nameElement.str());

  return boost::shared_ptr<chemkit::Molecule>(molecule);
}

// --- Annotations --------------------------------------------------------- //
/// Inserts a new annotation for the molecule refered to by \p ref.
void MongoDatabase::addAnnotation(const MoleculeRef &ref,
                                  const std::string &comment)
{
  if (!ref.isValid())
    return;

  // add new annotation
  mongo::BSONObjBuilder annotation;
  annotation.append("user", "unknown");
  annotation.append("comment", comment);

  // store annotations
  m_db->update(moleculesCollectionName(),
               QUERY("_id" << ref.id()),
               BSON("$push" << BSON("annotations" << annotation.obj())),
               false,
               true);
}

/// Deletes the annotation at \p index in the molecule refered to by \ref.
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

/// Updates the comment for the annotation at \p index in the molecule refered
/// to by \ref.
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

// --- Tags ---------------------------------------------------------------- //
/// Adds a new tag to the molecule refered to by \p ref.
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

/// Removes the given tag from the molecule refered to by \p ref.
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

/// Returns a vector of tags for the molecule refered to by \p ref.
std::vector<std::string> MongoDatabase::fetchTags(const MoleculeRef &ref)
{
  std::vector<std::string> tags;

  try {
    mongo::BSONObj obj = fetchMolecule(ref);
    boost::transform(obj["tags"].Array(),
                     std::back_inserter(tags),
                     boost::bind(&mongo::BSONElement::str, _1));
  }
  catch (mongo::UserException) {
  }

  return tags;
}

/// Returns a vector of all tags for \p collection that start with \p prefix.
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
  if(limit == 0)
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

    try {
      std::vector<mongo::BSONElement> array = obj["tags"].Array();

      for(size_t i = 0; i < array.size() && tags.size() < limit; i++){
        std::string tag = array[i].str();
        if(boost::starts_with(tag, prefix))
          tags.insert(tag);
      }
    }
    catch (mongo::UserException) {
    }
  }

  // return as a vector
  return std::vector<std::string>(tags.begin(), tags.end());
}

// --- Internal Methods ---------------------------------------------------- //
/// Returns the name of the molecules collection.
std::string MongoDatabase::moleculesCollectionName() const
{
  QSettings settings;

  QString collection = settings.value("collection", "chem").toString();

  return collection.toStdString() + ".molecules";
}

/// Creates a molecule ref using the object ID of \p obj.
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
