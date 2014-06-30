/******************************************************************************

  This source file is part of the MongoChem project.

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
#include <QtCore/QSize>
#include <QtCore/QDebug>
#include <QtGui/QColor>
#include <QtGui/QPixmap>
#include <QtCore/QSettings>

#include <vtkNew.h>
#include <vtkTable.h>
#include <vtkFloatArray.h>
#include <vtkStringArray.h>

#include "mongodatabase.h"

using namespace mongo;

namespace MongoChem {

class MongoModel::Private
{
public:
  BSONObj * getRecord(unsigned int index)
  {
    if (index < m_rowObjects.size()) {
      return &m_rowObjects[index];
    }
//    else {
//      while (index >= m_rowObjects.size() && m_cursor->more())
//        m_rowObjects.push_back(m_cursor->next().getOwned());
//      // If there are enough objects, return the one requested
//      if (index < m_rowObjects.size())
//        return &m_rowObjects[index];
//      else
//        return 0;
//    }
    return 0;
  }

  std::vector<BSONObj> m_rowObjects;

  QStringList m_fields;
  QMap<QString, QString> m_titles;
  DBClientConnection *db;
  auto_ptr<DBClientCursor> cursor;
  mongo::Query m_query;
  std::string m_sortField;
  int m_sortDirection;
};

MongoModel::MongoModel(mongo::DBClientConnection *db, QObject *parent_)
  : QAbstractItemModel(parent_)
{
  d = new MongoModel::Private;
  d->db = db;

  // Show the entire database by default.
  setQuery(QUERY("diagram" << BSON("$exists" << true)));

  d->m_fields << "diagram"
              << "name"
              << "formula"
              << "mass";
  d->m_titles["diagram"] = tr("Diagram");
  d->m_titles["name"] = tr("Name");
  d->m_titles["formula"] = tr("Formula");
  d->m_titles["mass"] = tr("Mass");
}

MongoModel::~MongoModel()
{
  delete d;
}

void MongoModel::setQuery(const mongo::Query &query)
{
  d->m_rowObjects.clear();

  // Store the query.
  d->m_query = query;

  try {
    QSettings settings;
    std::string collection =
        settings.value("collection", "chem").toString().toStdString();

    if (d->m_sortField.empty()) {
      d->cursor = d->db->query(collection + ".molecules", query);
    }
    else {
      // Add sort criteria to query.
      mongo::Query sortQuery = query;
      sortQuery.sort(d->m_sortField, d->m_sortDirection);
      d->cursor = d->db->query(collection + ".molecules", sortQuery);
    }

    // Load the first 50 rows.
    loadMoreData(50);
  }
  catch (mongo::SocketException &e) {
    std::cerr << "Failed to query MongoDB: " << e.what() << std::endl;
  }
}

void MongoModel::setSortField(const std::string &field, int direction)
{
  if (field == "diagram" || field == "3d") {
    // Sorting is not supported for diagrams or 3d coordinate.
    return;
  }

  d->m_sortField = field;
  if (field == "mass")
    d->m_sortField = "descriptors.mass";
  d->m_sortDirection = direction;

  // Re-run current query with new sorting parameters.
  setQuery(d->m_query);
}

void MongoModel::setSortColumn(int index_, int direction)
{
  if (index_ == -1) {
    d->m_sortField.clear();

    // re-run current query without sorting
    setQuery(d->m_query);
  }
  else if (index_ >= d->m_fields.size()) {
    return;
  }
  else {
    std::string field = d->m_fields[index_].toStdString();
    setSortField(field, direction);
  }
}

void MongoModel::addFieldColumn(const QString &name)
{
  if (d->m_fields.contains(name))
    return;

  emit layoutAboutToBeChanged();

  d->m_fields.append(name);
  d->m_titles[name] = name;

  emit layoutChanged();
}

void MongoModel::removeFieldColumn(int index_)
{
  if (index_ < 0 || index_ >= d->m_fields.size())
    return;

  emit layoutAboutToBeChanged();

  QString name = d->m_fields[index_];
  d->m_fields.removeAt(index_);
  d->m_titles.remove(name);

  emit layoutChanged();
}

QModelIndex MongoModel::parent(const QModelIndex &) const
{
  return QModelIndex();
}

int MongoModel::rowCount(const QModelIndex &parent_) const
{
  Q_UNUSED(parent_);

  return static_cast<int>(d->m_rowObjects.size());
}

int MongoModel::columnCount(const QModelIndex &parent_) const
{
  Q_UNUSED(parent_);

  return d->m_fields.size();
}

QVariant MongoModel::headerData(int section, Qt::Orientation orientation,
                                int role) const
{
  if (section >= d->m_titles.size()) {
    return QVariant();
  }
  else if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
    if (d->m_titles.contains(d->m_fields[section]))
      return d->m_titles[d->m_fields[section]];
    else
      return d->m_fields[section];
  }
  else if (orientation == Qt::Vertical && role == Qt::DisplayRole) {
    return QVariant(section);
  }
  return QVariant();
}

QVariant MongoModel::data(const QModelIndex &index_, int role) const
{
  if (index_.column() < 0 || index_.column() >= d->m_fields.size())
    return QVariant();

  BSONObj *obj = static_cast<BSONObj *>(index_.internalPointer());
  if (obj) {
    if (role == Qt::DisplayRole) {
      if (d->m_fields[index_.column()] == "mass") {
        BSONElement mass = obj->getObjectField("descriptors").getField("mass");
        if (!mass.eoo() && !mass.isNull() && mass.isNumber())
          return QVariant(mass.number());
        else
          return QVariant();
      }
      else {
        BSONElement e = obj->getField(d->m_fields[index_.column()].toStdString());
        if (e.eoo() || e.isNull())
          return QVariant();
        else if (e.isNumber())
          return QVariant(e.number());
        else
          return e.str().c_str();
      }
    }
    else if (role == Qt::SizeHintRole) {
      if (d->m_fields[index_.column()] == "diagram"
          && !obj->getObjectField("diagram").getField("png").eoo()){
        return QVariant(QSize(250, 250));
      }
      else {
        return QVariant(QSize(120, 20));
      }
    }
    else if (role == Qt::DecorationRole) {
      if (d->m_fields[index_.column()] == "diagram") {
        BSONElement image = obj->getObjectField("diagram").getField("png");
        if (!image.eoo()) {
          int length = 0;
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

bool MongoModel::setData(const QModelIndex &index_,
                         const QVariant &value,
                         int role)
{
  Q_UNUSED(index_);
  Q_UNUSED(value);
  Q_UNUSED(role);
  return false;
}

Qt::ItemFlags MongoModel::flags(const QModelIndex &index_) const
{
  Q_UNUSED(index_);

  return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QModelIndex MongoModel::index(int row, int column,
                              const QModelIndex &parent_) const
{
  Q_UNUSED(parent_);

  if (row >= 0 && static_cast<size_t>(row) < d->m_rowObjects.size())
    return createIndex(row, column, &d->m_rowObjects[row]);

  return QModelIndex();
}

void MongoModel::setMolecules(const std::vector<MoleculeRef> &molecules_)
{
  MongoDatabase *db = MongoDatabase::instance();

  d->m_rowObjects.clear();

  for (size_t i = 0; i < molecules_.size(); i++)
    d->m_rowObjects.push_back(db->fetchMolecule(molecules_[i]));
}

/// Returns a vector containing a reference to each molecule in the model.
std::vector<MoleculeRef> MongoModel::molecules() const
{
  std::vector<MoleculeRef> molecules_;

  for (size_t i = 0; i < d->m_rowObjects.size(); ++i) {
    const mongo::BSONObj &obj = d->m_rowObjects[i];
    mongo::BSONElement idElement;
    if (obj.getObjectID(idElement))
      molecules_.push_back(MoleculeRef(idElement.OID().str()));
  }

  return molecules_;
}

void MongoModel::clear()
{
}

bool MongoModel::setImage2D(int row, const QByteArray &image)
{
  emit layoutAboutToBeChanged();

  BSONObj *obj = d->getRecord(row);
  BSONObjBuilder b;
  b.appendBinData("diagram", image.length(), mongo::BinDataGeneral,
                  image.data());
  BSONObjBuilder updateSet;
  updateSet << "$set" << b.obj();
  d->db->update("chem.molecules", *obj, updateSet.obj());

  BSONElement id;
  obj->getObjectID(id);
  *obj = d->db->findOne("chem.molecules", QUERY("_id" << id));

  emit layoutChanged();

  return true;
}

bool MongoModel::hasMoreData() const
{
  return d->cursor.get() && d->cursor->more();
}

void MongoModel::loadMoreData(int count)
{
  if(!d->cursor.get())
    return;

  emit layoutAboutToBeChanged();

  size_t initial = d->m_rowObjects.size();

  while(d->cursor->more() && count--)
    d->m_rowObjects.push_back(d->cursor->next().copy());

  // TODO: show this in the status bar rather than the terminal
  std::cout << "Loaded "
            << d->m_rowObjects.size() - initial
            << (initial ? " more" : "") << " rows"
            << std::endl;

  emit layoutChanged();
}

void MongoModel::sort(int column, Qt::SortOrder order)
{
  setSortColumn(column, order == Qt::AscendingOrder ? 1 : -1);
}

} // End of namespace
