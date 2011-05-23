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
  QTreeView *tree = new QTreeView(this);
  tree->setModel(m_model);
  layout->addWidget(tree);
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
  qDebug() << "set record called!";
  mongo::BSONObj *obj = static_cast<mongo::BSONObj *>(index.internalPointer());

  mongo::BSONElement inchi = obj->getField("InChI");
  mongo::BSONElement iupac = obj->getField("IUPAC");
  mongo::BSONElement cml = obj->getField("CML File");
  mongo::BSONElement png = obj->getField("2D PNG");

  m_model->setBSONObj(obj);

  qDebug() << index.row() << inchi.str().c_str();
}

void DetailDialog::showEvent(QShowEvent *event)
{
  QDialog::showEvent(event);
}

} // End namespace
