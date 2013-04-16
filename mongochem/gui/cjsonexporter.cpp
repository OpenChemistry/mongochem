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

#include <mongo/client/dbclient.h>
#include <fstream>

namespace MongoChem {

std::string CjsonExporter::toCjson(const mongo::BSONObj &mongoChemObj)
{
  std::vector<std::string> toCopy;
  toCopy.push_back("name");
  toCopy.push_back("inchi");
  toCopy.push_back("formula");
  toCopy.push_back("atoms");
  toCopy.push_back("bonds");
  toCopy.push_back("properties");

  mongo::BSONObjBuilder builder;

  for (size_t i = 0; i < toCopy.size(); i++) {

    mongo::BSONElement field = mongoChemObj.getField(toCopy[i]);

    if (!field.eoo())
      builder.append(field);
  }

  // Add the chemical JSON version field.
  builder.append("chemical json", 0);

  mongo::BSONObj obj = builder.obj();

  return obj.jsonString(mongo::Strict);
}
} /* namespace MongoChem */
