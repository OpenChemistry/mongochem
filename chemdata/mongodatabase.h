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

#ifndef MONGODATABASE_H
#define MONGODATABASE_H

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

#include <mongo/client/dbclient.h>

#include <chemkit/molecule.h>

#include "moleculeref.h"

class MongoDatabase
{
public:
  static MongoDatabase* instance();

  bool isConnected() const;
  mongo::DBClientConnection* connection() const;


  MoleculeRef findMoleculeFromIdentifer(const std::string &identifier,
                                        const std::string &format);
  MoleculeRef findMoleculeFromInChI(const std::string &inchi);
  MoleculeRef findMoleculeFromInChIKey(const std::string &inchikey);
  MoleculeRef findMoleculeFromBSONObj(const mongo::BSONObj *obj);

  mongo::BSONObj fetchMolecule(const MoleculeRef &molecule);
  std::vector<mongo::BSONObj> fetchMolecules(const std::vector<MoleculeRef> &molecules);
  boost::shared_ptr<chemkit::Molecule> createMolecule(const MoleculeRef &ref);

  void addAnnotation(const MoleculeRef &ref, const std::string &comment);
  void deleteAnnotation(const MoleculeRef &ref, size_t index);
  void updateAnnotation(const MoleculeRef &ref,
                        size_t index,
                        const std::string &comment);

private:
  MongoDatabase();
  ~MongoDatabase();

  std::string moleculesCollectionName() const;
  MoleculeRef createMoleculeRefForBSONObj(const mongo::BSONObj &obj) const;

private:
  mongo::DBClientConnection *m_db;
};

#endif // MONGODATABASE_H
