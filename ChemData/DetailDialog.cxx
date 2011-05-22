/******************************************************************************

  This source file is part of the ChemData project.

  Copyright 2011 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "DetailDialog.h"

#include <mongo/client/dbclient.h>

namespace ChemData {

DetailDialog::DetailDialog(QWidget *parent) : QDialog(parent), m_model(0),
  m_row(-1)
{

}

} // End namespace
