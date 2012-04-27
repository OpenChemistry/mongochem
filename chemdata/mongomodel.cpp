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

using namespace mongo;

namespace ChemData {

class MongoModel::Private
{
public:
  Private()
  {
  }

  ~Private()
  {
  }

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
};

MongoModel::MongoModel(mongo::DBClientConnection *db, QObject *parent)
  : QAbstractItemModel(parent)
{
  d = new MongoModel::Private;
  d->db = db;

  // show entire database by default
  setQuery(Query());

  d->m_fields << "diagram"
              << "name"
              << "formula"
              << "mass";
  d->m_titles["diagram"] = "Diagram";
  d->m_titles["name"] = "Name";
  d->m_titles["formula"] = "Formula";
  d->m_titles["mass"] = "Molecular Mass";
}

MongoModel::~MongoModel()
{
  delete d;
}

void MongoModel::setQuery(const mongo::Query &query)
{
  d->m_rowObjects.clear();

  try {
    QSettings settings;
    std::string collection =
        settings.value("collection", "chem").toString().toStdString();
    d->cursor = d->db->query(collection + ".molecules", query);

    while(d->cursor->more()){
      d->m_rowObjects.push_back(d->cursor->next().copy());
    }

    qDebug() << "Loaded: " << d->m_rowObjects.size() << "rows";
  }
  catch (mongo::SocketException &e) {
    std::cerr << "Failed to query MongoDB: " << e.what() << std::endl;
  }
}

QModelIndex MongoModel::parent(const QModelIndex &) const
{
  return QModelIndex();
}

int MongoModel::rowCount(const QModelIndex &parent) const
{
  return d->m_rowObjects.size();
}

int MongoModel::columnCount(const QModelIndex &parent) const
{
  return d->m_fields.size();
}

QVariant MongoModel::headerData(int section, Qt::Orientation orientation,
                                int role) const
{
  if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
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

QVariant MongoModel::data(const QModelIndex &index, int role) const
{
  BSONObj *obj = static_cast<BSONObj *>(index.internalPointer());
  if (obj) {
    if (role == Qt::DisplayRole) {
      BSONElement e = obj->getField(d->m_fields[index.column()].toStdString());
      if (e.eoo() || e.isNull())
        return QVariant();
      else if (e.isNumber())
        return QVariant(e.number());
      else
        return e.str().c_str();
    }
    else if (role == Qt::SizeHintRole) {
      if(d->m_fields[index.column()] == "diagram" &&
         !obj->getField("diagram").eoo()){
        return QVariant(QSize(250, 250));
      }
      else{
        return QVariant(QSize(120, 20));
      }
    }
    else if(role == Qt::DecorationRole){
      if(d->m_fields[index.column()] == "diagram"){
        BSONElement image = obj->getField("diagram");
        if(!image.eoo()){
          int length = 0;
          const char *data = image.binData(length);
          QByteArray inData(data, length);
          QImage in = QImage::fromData(inData, "PNG");
          QPixmap pix = QPixmap::fromImage(in);
          return QVariant(pix);
        }
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
  return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

QModelIndex MongoModel::index(int row, int column,
                              const QModelIndex &parent) const
{
  if (row >= 0 && row < d->m_rowObjects.size()) {
    return createIndex(row, column, &d->m_rowObjects[row]);
  }

  return QModelIndex();
}

QStringList MongoModel::moleculeInChIs() const
{
  QStringList inchis;

  for (int i = 0; i < d->m_rowObjects.size(); i++) {
    BSONElement inchiElement = d->m_rowObjects[i].getField("inchi");
    if(!inchiElement.eoo()) {
      std::string inchi = inchiElement.str();

      inchis.append(QString::fromStdString(inchi));
    }
  }

  return inchis;
}

void MongoModel::clear()
{
}

bool MongoModel::setImage2D(int row, const QByteArray &image)
{
  emit layoutAboutToBeChanged();

  BSONObj *obj = d->getRecord(row);
  BSONObjBuilder b;
  b.appendBinData("diagram", image.length(), mongo::BinDataGeneral, image.data());
  BSONObjBuilder updateSet;
  updateSet << "$set" << b.obj();
  d->db->update("chem.molecules", *obj, updateSet.obj());

  BSONElement id;
  obj->getObjectID(id);
  *obj = d->db->findOne("chem.molecules", QUERY("_id" << id));

  emit layoutChanged();

  return true;
}

} // End of namespace
