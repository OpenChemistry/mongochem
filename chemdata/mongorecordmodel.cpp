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

#include "mongorecordmodel.h"

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
  std::vector<std::string> m_fields;
};

MongoRecordModel::MongoRecordModel(QObject *parent_)
  : QAbstractItemModel(parent_), d(new Private(0))
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

int MongoRecordModel::rowCount(const QModelIndex &parent_) const
{
  int rows;
  if (!parent_.isValid())
    rows =  d->m_fields.size();
  else
    rows = 0;
  return rows;
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

QVariant MongoRecordModel::data(const QModelIndex &index_, int role) const
{
  if (d->m_obj) {
    if (role == Qt::DisplayRole) {
      BSONElement e = d->m_obj->getField(d->m_fields[index_.row()]);
      if (index_.column() == 0)
        return QVariant(QString(d->m_fields[index_.row()].c_str()));
      else {
        if (e.isNumber())
          return QVariant(e.number());
        else if (e.isNull())
          return QVariant();
        else
          return e.str().c_str();
      }
    }
/*    else if (role == Qt::SizeHintRole) {
      if (d->m_fields[index.column()] == "diagram" &&
          !d->m_obj->getField("diagram").eoo()) {
        return QVariant(QSize(250, 250));
      }
      else {
        return QVariant(QSize(120, 20));
      }
    } */
    else if (role == Qt::DecorationRole) {
      if (d->m_fields[index_.row()] == "diagram" && index_.column() == 1) {
        BSONElement image = d->m_obj->getField("diagram");
        if (!image.eoo()) {
          int length;
          const char *data_ = image.binData(length);
          QByteArray inData(data_, length);
          QImage in = QImage::fromData(inData, "PNG");
          QPixmap pix = QPixmap::fromImage(in);
          return QVariant(pix);
        }
      }
    }
  }
  return QVariant();
}

bool MongoRecordModel::setData(const QModelIndex &index_, const QVariant &value,
                         int role)
{
  Q_UNUSED(index_);
  Q_UNUSED(value);
  Q_UNUSED(role);

  return false;
}

Qt::ItemFlags MongoRecordModel::flags(const QModelIndex &index_) const
{
  if (index_.column() == 0)
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
  else
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QModelIndex MongoRecordModel::index(int row, int column,
                              const QModelIndex &parent_) const
{
  Q_UNUSED(parent_);

  if (row >= 0 && static_cast<size_t>(row) < d->m_fields.size()) {
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
  // It is much simpler to enforce an ordered list so we can access fields for display
  d->m_fields.clear();;
  std::set<std::string> fields;
  d->m_obj->getFieldNames(fields);
  for (std::set<std::string>::const_iterator i = fields.begin();
       i != fields.end(); ++i) {
    d->m_fields.push_back(*i);
  }
  reset();
}

} // End of namespace
