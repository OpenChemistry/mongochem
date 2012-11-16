/******************************************************************************

  This source file is part of the ChemData project.

  Copyright 2011-2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "selectionfiltermodel.h"

#include <vtkSelection.h>
#include <vtkIdTypeArray.h>
#include <vtkSelectionNode.h>

#include <QDebug>

SelectionFilterModel::SelectionFilterModel(QObject *parent_)
  : QSortFilterProxyModel(parent_)
{
}

SelectionFilterModel::~SelectionFilterModel()
{
}

void SelectionFilterModel::setSelection(vtkSelection *selection)
{
  m_selection = selection;
}

bool SelectionFilterModel::filterAcceptsColumn(int source_column,
                                               const QModelIndex &source_parent) const
{
  Q_UNUSED(source_column);
  Q_UNUSED(source_parent);

  // accept all columns
  return true;
}

bool SelectionFilterModel::filterAcceptsRow(int source_row,
                                            const QModelIndex &source_parent) const
{
  Q_UNUSED(source_parent);

  if (m_selection->GetNumberOfNodes() < 1)
    return false;

  vtkSelectionNode *node = m_selection->GetNode(0);
  if (!node)
    return false;

  vtkIdTypeArray *selectionArray =
    vtkIdTypeArray::SafeDownCast(node->GetSelectionList());
  if (!selectionArray)
    return false;

  return selectionArray->LookupValue(static_cast<vtkIdType>(source_row)) != -1;
}
