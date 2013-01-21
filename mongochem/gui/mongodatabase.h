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

#ifndef MONGODATABASE_H
#define MONGODATABASE_H

#include "mongochemguiexport.h"
#include "moleculeref.h"

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <mongo/client/dbclient.h>

#include <chemkit/molecule.h>

namespace MongoChem {

/**
 * \class MongoDatabase
 * \brief The MongoDatabase class represents a connection to a
 *        Mongo database.
 *
 * The MongoDatabase class is implemented as a singleton. The static
 * instance() method is used to retrieve a handle to the database.
 *
 * The find*() methods in this class allow users to query for molecules
 * using various identifiers and return MoleculeRef objects representing
 * the molecules found.
 *
 * The fetch*() methods take MoleculeRef's and return BSONObj's containing
 * the corresponding molecular data.
 *
 * \warning The first invocation of \p instance() forms a persistant
 *          connection to the mongo database. This method is not reentrant
 *          and should be called only from a single thread.
 */

class MONGOCHEMGUI_EXPORT MongoDatabase
{
public:
  /** Returns an instance of the singleton mongo database. */
  static MongoDatabase* instance();

  /** Disconnect from the currently connected MongoDB server. */
  void disconnect();

  /**
   * Returns \c true if the database object is connected to the mongo database
   * server.
   */
  bool isConnected() const;

  /** Returns the connection to the mongo database. */
  mongo::DBClientConnection* connection() const;

  /** Returns the current user name. */
  std::string userName() const;

  /**
   * Queries the database for a molecule molecule with \p identifier in \p
   * format.
   */
  MoleculeRef findMoleculeFromIdentifier(const std::string &identifier,
                                         const std::string &format);

  /** Returns a molecule ref corresponding to the molecule with \p inchi. */
  MoleculeRef findMoleculeFromInChI(const std::string &inchi);

  /** Returns a molecule ref corresponding to the molecule with \p inchikey. **/
  MoleculeRef findMoleculeFromInChIKey(const std::string &inchikey);

  /**
   * Returns a molecule ref corresponding to the molecule represented by \p obj.
   */
  MoleculeRef findMoleculeFromBSONObj(const mongo::BSONObj *obj);

  /**
   * Creates a new molecule for @p identifier with @p format. Returns a
   * reference to the newly created molecule.
   *
   * If the molecule already exists in the database no action is performed
   * and a reference to the existing molecule is returned.
   *
   * The following line formats are supported:
   *   - InChI
   *   - SMILES
   *
   * If @p format is not supported a null reference is returned.
   */
  MoleculeRef importMoleculeFromIdentifier(const std::string &identifier,
                                           const std::string &format);

  /**
   * Returns a BSONObj containing the data for the molecule referenced by \p
   * molecule.
   */
  mongo::BSONObj fetchMolecule(const MoleculeRef &molecule);

  /**
   * Returns a vector of BSONObj's containing the data for the molecules
   * referenced by \p molecules.
   */
  std::vector<mongo::BSONObj> fetchMolecules(const std::vector<MoleculeRef> &molecules);

  /**
   * Creates a new molecule object for \p ref. The ownership of the returned
   * molecule object is passed to the caller.
   */
  boost::shared_ptr<chemkit::Molecule> createMolecule(const MoleculeRef &ref);

  /**
   * Set the \p property for the molecule contained in \p ref to \p value. This
   * sets the property immediately in the database.
   */
  template<class T>
  void setMoleculeProperty(const MoleculeRef &ref,
                           const std::string &property,
                           const T &value)
  {
    m_db->update(moleculesCollectionName(),
                 QUERY("_id" << ref.id()),
                 BSON("$set" << BSON(property << value)),
                 true,
                 true);
  }

  /** Inserts a new annotation for the molecule refered to by \p ref. */
  void addAnnotation(const MoleculeRef &ref, const std::string &comment);

  /** Deletes the annotation at \p index in the molecule refered to by \p ref. */
  void deleteAnnotation(const MoleculeRef &ref, size_t index);

  /**
   * Updates the comment for the annotation at \p index in the molecule refered
   * to by \p ref.
   */
  void updateAnnotation(const MoleculeRef &ref,
                        size_t index,
                        const std::string &comment);

  /** Adds a new tag to the molecule refered to by \p ref. */
  void addTag(const MoleculeRef &ref, const std::string &tag);

  /** Removes the given tag from the molecule refered to by \p ref. */
  void removeTag(const MoleculeRef &ref, const std::string &tag);

  /** Returns a vector of tags for the molecule refered to by \p ref. */
  std::vector<std::string> fetchTags(const MoleculeRef &ref);

  /** Returns a vector of all tags for \p collection that start with \p prefix. */
  std::vector<std::string> fetchTagsWithPrefix(const std::string &collection,
                                               const std::string &prefix,
                                               size_t limit = 0);

private:
  /**
   * Creates a new mongo database object. This constructor should not
   * be called by users; rather the instance() method should be used
   * to retrieve a handle to the mongo database.
   */
  MongoDatabase();

  ~MongoDatabase();

  /** Returns the name of the molecules collection. */
  std::string moleculesCollectionName() const;

  /** Creates a molecule ref using the object ID of \p obj. */
  MoleculeRef createMoleculeRefForBSONObj(const mongo::BSONObj &obj) const;

private:
  mongo::DBClientConnection *m_db;
};

} // end MongoChem namespace

#endif // MONGODATABASE_H
