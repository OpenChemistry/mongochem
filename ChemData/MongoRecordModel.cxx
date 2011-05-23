/******************************************************************************

  This source file is part of the MongoRecordModel project.

  Copyright 2011 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "MongoRecordModel.h"

#include <mongo/client/dbclient.h>
#include <mongo/client/gridfs.h>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QSize>
#include <QtCore/QDebug>
#include <QtGui/QColor>
#include <QtGui/QPixmap>

using namespace mongo;

namespace ChemData {

class MongoRecordModel::Private
{
public:
  Private(DBClientConnection *db = 0) : m_obj(0)
  {
    m_db = db;
  }

  ~Private()
  {
  }

  DBClientConnection *m_db;
  BSONObj *m_obj;
};

MongoRecordModel::MongoRecordModel(QObject *parent)
  : QAbstractItemModel(parent), d(new Private(0))
{
}

MongoRecordModel::~MongoRecordModel()
{
  delete d;
}

QModelIndex MongoRecordModel::parent(const QModelIndex &) const
{
  return QModelIndex();
}

int MongoRecordModel::rowCount(const QModelIndex &parent) const
{
  if (parent.isValid())
    return d->m_obj ? d->m_obj->nFields() : 0;
  else
    return 0;
}

int MongoRecordModel::columnCount(const QModelIndex &) const
{
  return 2;
}

QVariant MongoRecordModel::headerData(int section, Qt::Orientation orientation,
                                      int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    if (section == 0)
      return "Field";
    else if (section == 1)
      return "Value";
  }
  else if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
    return QVariant(section);
  }
  return QVariant();
}

QVariant MongoRecordModel::data(const QModelIndex &index, int role) const
{
  if (index.internalPointer()) {
    BSONObj *obj = static_cast<BSONObj *>(index.internalPointer());
  }
  return QVariant();
}

bool MongoRecordModel::setData(const QModelIndex &index, const QVariant &value,
                         int role)
{
  return false;
}

Qt::ItemFlags MongoRecordModel::flags(const QModelIndex &index) const
{
  if (index.column() == 0)
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
  else
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QModelIndex MongoRecordModel::index(int row, int column,
                              const QModelIndex &parent) const
{
  if (row >= 0 && row < 9) {
    return createIndex(row, column, 0);
  }
  return QModelIndex();
}

void MongoRecordModel::clear()
{
}

void MongoRecordModel::setBSONObj(mongo::BSONObj *obj)
{
  d->m_obj = obj;
}

} // End of namespace
