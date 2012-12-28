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

#ifndef DIAGRAMTOOLTIPITEM_H
#define DIAGRAMTOOLTIPITEM_H

#include "mongochemguiexport.h"
#include "vtkTooltipItem.h"

#include <mongo/client/dbclient.h>

namespace MongoChem {

class MONGOCHEMGUI_EXPORT DiagramTooltipItem : public vtkTooltipItem
{
public:
  vtkTypeMacro(DiagramTooltipItem, vtkTooltipItem);
  virtual void PrintSelf(ostream &os, vtkIndent indent);

  // Creates a 2D Chart object.
  static DiagramTooltipItem *New();

  bool Paint(vtkContext2D *painter);

protected:
  DiagramTooltipItem();

private:
  mongo::DBClientConnection m_db;
};

} // end MongoChem namespace

#endif // DIAGRAMTOOLTIPITEM_H
