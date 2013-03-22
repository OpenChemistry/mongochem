/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2011-2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef MONGOCHEM_SELECTIONFILTERMODEL_H
#define MONGOCHEM_SELECTIONFILTERMODEL_H

#include <QSortFilterProxyModel>

#include <vtkSelection.h>

namespace MongoChem {

class SelectionFilterModel : public QSortFilterProxyModel
{
  Q_OBJECT

public:
  explicit SelectionFilterModel(QObject *parent = 0);
  ~SelectionFilterModel();

  void setSelection(vtkSelection *selection);

protected:
  bool filterAcceptsColumn(int source_column,
                           const QModelIndex &source_parent) const;
  bool filterAcceptsRow(int source_row, const QModelIndex &source_parent) const;

private:
  vtkSelection *m_selection;
};

} // end MongoChem namespace

#endif // MONGOCHEM_SELECTIONFILTERMODEL_H
