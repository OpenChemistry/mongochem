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

#include "MongoRecordModel.h"

#include <mongo/client/dbclient.h>

#include <QtCore/QDebug>
#include <QtCore/QModelIndex>
#include <QtGui/QVBoxLayout>
#include <QtGui/QTreeView>

namespace ChemData {

DetailDialog::DetailDialog(QWidget *parent) : QDialog(parent),
  m_model(new MongoRecordModel(this)),
  m_row(-1)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  m_treeView = new QTreeView(this);
  m_treeView->setModel(m_model);
  layout->addWidget(m_treeView);
  setLayout(layout);
}

void DetailDialog::setModel(MongoModel *model)
{

}

void DetailDialog::setRow(int row)
{
  m_row = row;
}

void DetailDialog::setActiveRecord(const QModelIndex &index)
{
  mongo::BSONObj *obj = static_cast<mongo::BSONObj *>(index.internalPointer());

  m_model->setBSONObj(obj);
}

void DetailDialog::showEvent(QShowEvent *event)
{
  QDialog::showEvent(event);
}

} // End namespace
