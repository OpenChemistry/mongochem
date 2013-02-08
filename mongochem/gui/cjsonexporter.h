/******************************************************************************

 This source file is part of the MoleQueue project.

 Copyright 2013 Kitware, Inc.

 This source code is released under the New BSD License, (the "License").

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.

 ******************************************************************************/

#ifndef MONGOCHEM_GUI_CJSONEXPORTER_H
#define MONGOCHEM_GUI_CJSONEXPORTER_H

#include <string>

namespace mongo {
class BSONObj;
}

namespace MongoChem
{
/**
 * @class CjsonExporter cjsonexporter.h <mongochem/gui/cjsonexporter.h>
 * @brief Utility class to convert from mongochem BSON into CJSON.
 */
class CjsonExporter
{
public:

  /**
   * Converts a MongoChem database object into CJSON.
   *
   * @para mongoChemJson The database object.
   * @return A CJSON string for the molecule.
   */
  static std::string toCjson(const mongo::BSONObj &mongoChemObj);

};

} /* namespace MongoChem */

#endif /* MONGOCHEM_GUI_CJSONEXPORTER_H */
