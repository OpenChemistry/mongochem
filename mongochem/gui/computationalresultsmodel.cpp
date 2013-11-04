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

#include <mongo/client/dbclient.h>
#include <mongo/client/gridfs.h>

#include "mongodatabase.h"

namespace MongoChem {

ComputationalResultsModel::ComputationalResultsModel(QObject *parent_)
  : QAbstractItemModel(parent_)
{
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
      db->connection()->query(collection + ".quantum", query);

    // Let's read in the results, but limit to 1000 in case of bad queries.
    int i(0);
    while (cursor->more() && ++i < 1000)
      m_objects.push_back(cursor->next().copy());
  }
  catch (mongo::SocketException &e) {
    std::cerr << "Failed to query MongoDB: " << e.what() << std::endl;
  }

  endInsertRows();
}

QModelIndex ComputationalResultsModel::index(int row,
                                             int column,
                                             const QModelIndex &) const
{
  if (row >= 0 && static_cast<size_t>(row) < m_objects.size())
    return createIndex(row, column, (void *) &m_objects[row]);

  return QModelIndex();
}

QModelIndex ComputationalResultsModel::parent(const QModelIndex &) const
{
  return QModelIndex();
}

int ComputationalResultsModel::rowCount(const QModelIndex &) const
{
  return static_cast<int>(m_objects.size());
}

int ComputationalResultsModel::columnCount(const QModelIndex &) const
{
  return 5;
}

QVariant ComputationalResultsModel::data(const QModelIndex &index_, int role) const
{
  int row = index_.row();
  int column = index_.column();

  if (static_cast<size_t>(row) >= m_objects.size()) {
    // invalid row index
    return QVariant();
  }

  const mongo::BSONObj &obj = m_objects[row];
  if (role == Qt::DisplayRole) {
    switch (column) {
    case 0:
      return obj.getStringField("name");
    case 1:
      return obj.getStringField("program");
    case 2:
      return obj.getStringField("type");
    case 3:
      if (obj.hasField("calculation")
          && obj.getObjectField("calculation").hasField("theory")) {
        return obj.getObjectField("calculation").getStringField("theory");
      }
    case 4:
      if (obj.hasField("energy")
          && obj.getObjectField("energy").hasField("total")
          && obj.getObjectField("energy").getField("total").isNumber()) {
        return obj.getObjectField("energy").getField("total").Number();
      }
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

Qt::ItemFlags ComputationalResultsModel::flags(const QModelIndex &) const
{
  return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

} // end MongoChem namespace
