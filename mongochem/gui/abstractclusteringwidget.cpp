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

#include "abstractclusteringwidget.h"

namespace MongoChem {

AbstractClusteringWidget::AbstractClusteringWidget(QWidget *parent_)
  : QWidget(parent_)
{
}

void AbstractClusteringWidget::setMolecules(const std::vector<MoleculeRef> &molecules)
{
  Q_UNUSED(molecules);
}

} // end MongoChem namespace
