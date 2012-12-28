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

#include "computationalresultsmodel.h"

#include <QSettings>

#include <mongo/client/gridfs.h>

#include "mongodatabase.h"

namespace MongoChem {

ComputationalResultsModel::ComputationalResultsModel(QObject *parent_)
  : QAbstractItemModel(parent_)
{
  // show entire database by default
  setQuery(mongo::Query());
}

ComputationalResultsModel::~ComputationalResultsModel()
{
}

void ComputationalResultsModel::setQuery(const mongo::Query &query)
{
  beginInsertRows(QModelIndex(), 0, rowCount(QModelIndex()));

  m_objects.clear();

  MongoDatabase *db = MongoDatabase::instance();

  try {
    QSettings settings;
    std::string collection =
        settings.value("collection", "chem").toString().toStdString();
    std::auto_ptr<mongo::DBClientCursor> cursor =
      db->connection()->query(collection + ".jobs", query);

    while (cursor->more()) {
      m_objects.push_back(cursor->next().copy());
    }
  }
  catch (mongo::SocketException &e) {
    std::cerr << "Failed to query MongoDB: " << e.what() << std::endl;
  }

  endInsertRows();
}

QModelIndex ComputationalResultsModel::index(int row,
                                             int column,
                                             const QModelIndex &parent_) const
{
  Q_UNUSED(parent_);

  if (row >= 0 && static_cast<size_t>(row) < m_objects.size())
    return createIndex(row, column, (void *) &m_objects[row]);

  return QModelIndex();
}

QModelIndex ComputationalResultsModel::parent(const QModelIndex &child) const
{
  Q_UNUSED(child);

  return QModelIndex();
}

int ComputationalResultsModel::rowCount(const QModelIndex &parent_) const
{
  Q_UNUSED(parent_);

  return static_cast<int>(m_objects.size());
}

int ComputationalResultsModel::columnCount(const QModelIndex &parent_) const
{
  Q_UNUSED(parent_);

  return 5;
}

QVariant ComputationalResultsModel::data(const QModelIndex &index_, int role) const
{
  Q_UNUSED(index_);
  Q_UNUSED(role);

  int row = index_.row();
  int column = index_.column();

  if (static_cast<size_t>(row) >= m_objects.size()) {
    // invalid row index
    return QVariant();
  }

  const mongo::BSONObj &obj = m_objects[row];
  if (role == Qt::DisplayRole) {
    switch(column){
    case 0:
      return obj.getStringField("name");
    case 1:
      return obj.getStringField("program");
    case 2:
      return obj.getStringField("type");
    case 3:
      return obj.getStringField("theory");
    case 4:
      return obj.getIntField("energy");
    }
  }

  return QVariant();
}

QVariant ComputationalResultsModel::headerData(int section,
                                               Qt::Orientation orientation,
                                               int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    switch (section) {
    case 0:
      return "Name";
    case 1:
      return "Program";
    case 2:
      return "Type";
    case 3:
      return "Theory";
    case 4:
      return "Energy";
    }
  }
  else if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
    return QVariant(section + 1);
  }

  return QVariant();
}

Qt::ItemFlags ComputationalResultsModel::flags(const QModelIndex &index_) const
{
  Q_UNUSED(index_);

  return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

} // end MongoChem namespace
