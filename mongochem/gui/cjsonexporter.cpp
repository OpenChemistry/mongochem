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

#include "cjsonexporter.h"

#include "mongodatabase.h"

#include <mongo/client/dbclient.h>

namespace MongoChem {

std::string CjsonExporter::toCjson(const mongo::BSONObj &mongoChemObj)
{
  // Follow the database link and convert to CJSON.
  MongoDatabase *db = MongoDatabase::instance();
  if (!db)
    return "";

  mongo::BSONObj structure = mongoChemObj.getObjectField("3dStructure");
  if (!structure.hasField("$ref") || !structure.hasField("$id")
      || !structure.getField("$id").isSimpleType()) {
    return "";
  }
  std::auto_ptr<mongo::DBClientCursor> cursor =
      db->query(db->databaseName() + "." + structure.getStringField("$ref"),
                QUERY("_id" << structure.getField("$id").OID()), 1);
  mongo::BSONObj object;
  if (cursor->more())
    object = cursor->next();
  else
    return "";

  std::vector<std::string> toCopy;
  toCopy.push_back("name");
  toCopy.push_back("inchi");
  toCopy.push_back("formula");
  toCopy.push_back("properties");
  mongo::BSONObjBuilder builder;

  for (size_t i = 0; i < toCopy.size(); i++) {
    mongo::BSONElement field = mongoChemObj.getField(toCopy[i]);
    if (!field.eoo())
      builder.append(field);
  }
  toCopy.clear();
  toCopy.push_back("atoms");
  toCopy.push_back("bonds");
  for (size_t i = 0; i < toCopy.size(); i++) {
    mongo::BSONElement field = object.getField(toCopy[i]);
    if (!field.eoo())
      builder.append(field);
  }

  // Add the chemical JSON version field.
  builder.append("chemical json", 0);

  mongo::BSONObj obj = builder.obj();

  return obj.jsonString(mongo::Strict);
}

} /* namespace MongoChem */
