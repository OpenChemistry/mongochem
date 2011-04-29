/******************************************************************************

  This source file is part of the MongoModel project.

  Copyright 2011 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "mongomodel.h"

#include <mongo/client/dbclient.h>
#include <mongo/client/gridfs.h>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QDebug>

using namespace mongo;

namespace ChemData {

class MongoModel::Private
{
public:
  Private(const QString &host)
  {
  this->connect(host);
  }

  ~Private()
  {

  }

  bool connect(const QString &host)
  {
    try {
      m_db.connect(host.toStdString());
      cout << "connected OK" << endl;

      m_rows = m_db.count("chem.import");
      m_cursor = m_db.query("chem.import");//, QUERY("ago" << 33));
      return true;
    }
    catch (DBException &e) {
      cout << "caught " << e.what() << endl;
      return false;
    }
  }

  BSONObj * getRecord(unsigned int index)
  {
    if (index < m_rowObjects.size()) {
      return &m_rowObjects[index];
    }
    else {
      while (index > m_rowObjects.size() && m_cursor->more())
        m_rowObjects.push_back(m_cursor->next());
      // If there are enough objects, return the one requested
      if (index < m_rowObjects.size())
        return &m_rowObjects[index];
      else
        return 0;
    }
  }

  DBClientConnection m_db;
  auto_ptr<DBClientCursor> m_cursor;
  std::vector<BSONObj> m_rowObjects;
  int m_rows;
};

MongoModel::MongoModel(QObject *parent)
  : QAbstractItemModel(parent)
{
  d = new MongoModel::Private("localhost");
}

QModelIndex MongoModel::parent(const QModelIndex &index) const
{
}

int MongoModel::rowCount(const QModelIndex &parent) const
{
  return d->m_rows;
}

int MongoModel::columnCount(const QModelIndex &parent) const
{
  return 2;
}

QVariant MongoModel::data(const QModelIndex &index, int role) const
{
  if (index.internalPointer()) {
    BSONObj *obj = static_cast<BSONObj *>(index.internalPointer());
    if (role == Qt::DisplayRole) {
      switch (index.column()) {
      case 0:
        return QVariant(QString(obj->getField("CAS").str().c_str()));
      case 1:
        return QVariant(QString(obj->getField("Set").str().c_str()));
      }
    }
  }
  return QVariant();
}

bool MongoModel::setData(const QModelIndex &index, const QVariant &value,
                             int role)
{
  return false;
}

Qt::ItemFlags MongoModel::flags(const QModelIndex &index) const
{
  if (index.column() == 0)
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
  else
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QModelIndex MongoModel::index(int row, int column,
                              const QModelIndex &parent) const
{
  if (row >= 0 && row < d->m_rows) {
    BSONObj *obj = d->getRecord(row);
    return createIndex(row, column, obj);
  }
  return QModelIndex();
}

void MongoModel::clear()
{
}

} // End of namespace
