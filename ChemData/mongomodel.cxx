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
#include <QtCore/QSize>
#include <QtCore/QDebug>
#include <QtGui/QColor>
#include <QtGui/QPixmap>

#include <vtkNew.h>
#include <vtkTable.h>
#include <vtkFloatArray.h>
#include <vtkStringArray.h>

using namespace mongo;

namespace ChemData {

class MongoModel::Private
{
public:
  Private(const QString &host) : m_namespace("chem.import"), m_rows(0)
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
      return query();
    }
    catch (DBException &e) {
      cout << "caught " << e.what() << endl;
      return false;
    }
  }

  bool query()
  {
    if (m_db.isFailed())
      return false;
    m_rowObjects.resize(0);
    m_rows = m_db.count(m_namespace.toStdString());
    m_rowObjects.reserve(m_rows);
    BSONObj empty;
    m_cursor = m_db.query(m_namespace.toStdString(), Query().sort("Observed"));
    //, QUERY("ago" << 33));
    return true;
  }

  BSONObj * getRecord(unsigned int index)
  {
    if (index < m_rowObjects.size()) {
      return &m_rowObjects[index];
    }
    else {
      while (index >= m_rowObjects.size() && m_cursor->more())
        m_rowObjects.push_back(m_cursor->next().getOwned());
      // If there are enough objects, return the one requested
      if (index < m_rowObjects.size())
        return &m_rowObjects[index];
      else
        return 0;
    }
  }

  DBClientConnection m_db;
  auto_ptr<DBClientCursor> m_cursor;
  QString m_namespace;
  std::vector<BSONObj> m_rowObjects;
  int m_rows;

  QStringList m_fields;
  QMap<QString, QString> m_titles;
};

MongoModel::MongoModel(QObject *parent)
  : QAbstractItemModel(parent)
{
  //d = new MongoModel::Private("localhost");
  d = new MongoModel::Private("londinium.kitwarein.com");

  d->m_fields << "CAS" << "Set"
              << "Molecular Weight"
              << "Observed"
              << "Predicted log Sw (MLR)"
              << "Predicted log Sw (RF)";
  d->m_titles["Observed"] = "Observed log(Sw)";
  d->m_titles["Predicted log Sw (MLR)"] = "log(Sw) (MLR)";
  d->m_titles["Predicted log Sw (RF)"] = "log(Sw) (RF)";
}

QModelIndex MongoModel::parent(const QModelIndex &) const
{
  return QModelIndex();
}

int MongoModel::rowCount(const QModelIndex &parent) const
{
  return d->m_rows;
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
  if (index.internalPointer()) {
    BSONObj *obj = static_cast<BSONObj *>(index.internalPointer());
    if (obj) {
      if (role == Qt::DisplayRole) {
        BSONElement e = obj->getField(d->m_fields[index.column()].toStdString());
        if (e.isNumber())
          return QVariant(e.number());
        else if (e.isNull())
          return QVariant();
        else
          return e.str().c_str();
      }
      else if (role == Qt::SizeHintRole) {
        if (obj->getField("2D PNG").eoo())
          return QVariant(QSize(10, 20));
        else
          return QVariant(QSize(250, 250));
      }
      else if (role == Qt::DecorationRole) {
        if (d->m_fields[index.column()] == "CAS") {
          if (obj->getField("InChIKey").eoo())
            return Qt::red;
          else
            return Qt::green;
        }
        else if (d->m_fields[index.column()] == "Molecular Weight") {
          BSONElement image = obj->getField("2D PNG");
          if (!image.eoo()) {
            int length;
            const char *data = image.binData(length);
            QByteArray inData(data, length);
            QImage in = QImage::fromData(inData, "PNG");
            QPixmap pix = QPixmap::fromImage(in);
            return QVariant(pix);
          }
        }
      }
      else if (role == Qt::ToolTipRole) {
        if (d->m_fields[index.column()] == "CAS") {
          BSONElement iupac = obj->getField("IUPAC");
          if (iupac.eoo())
            return "<b>Unknown</b>";
          else
            return iupac.str().c_str();
        }
        else if (d->m_fields[index.column()] == "Molecular Weight") {
          BSONElement e = obj->getField("Formula");
          if (!e.eoo()) {
            QStringList parts = QString(e.str().c_str()).split(" ",
                                                               QString::SkipEmptyParts);
            QString formula;
            for (int i = 0; i < parts.size(); i += 2) {
              formula += parts[i];
              if (parts[i+1].toInt() > 1)
                formula += "<sub>" + parts[i+1] + "</sub>";
            }
            return formula;
          }
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

bool MongoModel::addIdentifiers(int row, const QString &identifiers)
{
  BSONObj *obj = d->getRecord(row);
  BSONObjBuilder b;
  QStringList lines = identifiers.split("\n", QString::SkipEmptyParts);
  for (int i = 0; i < lines.size(); ++i) {
    if (lines[i].trimmed() == "[Formula]")
      b << "Formula" << lines[++i].trimmed().toStdString();
    else if (lines[i].trimmed() == "[Molecular weight]")
      b << "Molecular Weight" << lines[++i].trimmed().toDouble();
    else if (lines[i].trimmed() == "[smiles]")
      b << "SMILES" << lines[++i].trimmed().toStdString();
    else if (lines[i].trimmed() == "[canonical smiles]")
      b << "Canonical SMILES" << lines[++i].trimmed().toStdString();
    else if (lines[i].trimmed() == "[inchi]")
      b << "InChI" << lines[++i].trimmed().toStdString();
    else if (lines[i].trimmed() == "[inchikey]")
      b << "InChIKey" << lines[++i].trimmed().toStdString();
    else if (lines[i].trimmed() == "[XYZ]") {
      // Read in all of the XYZ file
      QString file = lines[++i] + "\n";
      while (lines[++i].trimmed() != "[end]")
        file += lines[i] + "\n";
      b << "XYZ File" << file.toStdString();
    }
    else if (lines[i].trimmed() == "[CML]") {
      // Read in all of the CML file
      QString file = lines[++i] + "\n";
      while (lines[++i].trimmed() != "[end]")
        file += lines[i] + "\n";
      b << "CML File" << file.toStdString();
    }
  }
  BSONObjBuilder updateSet;
  updateSet << "$set" << b.obj();
  d->m_db.update(d->m_namespace.toStdString(), *obj, updateSet.obj());
  d->query();
}

bool MongoModel::setIUPACName(int row, const QString &name)
{
  BSONObj *obj = d->getRecord(row);
  BSONObjBuilder b;
  b << "IUPAC" << name.toStdString();
  BSONObjBuilder updateSet;
  updateSet << "$set" << b.obj();
  d->m_db.update(d->m_namespace.toStdString(), *obj, updateSet.obj());
  d->query();
}

bool MongoModel::setImage2D(int row, const QByteArray &image)
{
  BSONObj *obj = d->getRecord(row);
  BSONObjBuilder b;
  qDebug() << "Image data:" << image.length() << image;
  b.appendBinData("2D PNG", image.length(), mongo::BinDataGeneral, image.data());
  BSONObjBuilder updateSet;
  updateSet << "$set" << b.obj();
  d->m_db.update(d->m_namespace.toStdString(), *obj, updateSet.obj());
  d->query();
}

bool MongoModel::addOutputFile(const QString &outputFile,
                               const QString &inputFile)
{
  qDebug() << "output" << outputFile << "input" << inputFile;
  BSONObjBuilder b;
  b.genOID();
  QFile output(outputFile);
  if (output.open(QIODevice::ReadOnly)) {
    QByteArray out = output.readAll();
    qDebug() << "GAMESS log file" << out;
    b.append("gamout", out.data());
    output.close();
  }
  else {
    qDebug() << "Failed to open output file:" << outputFile;
    return false;
  }
  if (!inputFile.isEmpty()) {
    QFile input(inputFile);
    if (input.open(QIODevice::ReadOnly)) {
      QByteArray in = input.readAll();
      b.append("gaminp", in.data());
      input.close();
    }
  }
  d->m_db.insert("chem.gamess", b.obj());
  return true;
}

bool MongoModel::results(vtkTable *table)
{
  vtkNew<vtkStringArray> CAS;
  CAS->SetName("CAS");
  CAS->SetNumberOfTuples(d->m_rows);
  vtkNew<vtkStringArray> set;
  set->SetName("Set");
  set->SetNumberOfTuples(d->m_rows);
  vtkNew<vtkFloatArray> observed;
  observed->SetName("Observed");
  observed->SetNumberOfTuples(d->m_rows);
  vtkNew<vtkFloatArray> mlr;
  mlr->SetName("log(Sw) (MLR)");
  mlr->SetNumberOfTuples(d->m_rows);
  vtkNew<vtkFloatArray> rf;
  rf->SetName("log(Sw) (RF)");
  rf->SetNumberOfTuples(d->m_rows);
  for (int i = 0; i < d->m_rows; ++i) {
    BSONObj *obj = d->getRecord(i);
    CAS->SetValue(i, obj->getField("CAS").str().c_str());
    set->SetValue(i, obj->getField("Set").str().c_str());
    observed->SetValue(i, obj->getField("Observed").number());
    mlr->SetValue(i, obj->getField("Predicted log Sw (MLR)").number());
    rf->SetValue(i, obj->getField("Predicted log Sw (RF)").number());
  }
  table->AddColumn(CAS.GetPointer());
  table->AddColumn(set.GetPointer());
  table->AddColumn(observed.GetPointer());
  table->AddColumn(mlr.GetPointer());
  table->AddColumn(rf.GetPointer());
  return true;
}

} // End of namespace
