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

#ifndef MongoModel_H
#define MongoModel_H

#include <QtCore/QModelIndex>
#include <QtCore/QList>

#include <mongo/client/dbclient.h>

class vtkTable;

namespace ChemData {

class MongoModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  explicit MongoModel(mongo::DBClientConnection *db, QObject *parent = 0);
  ~MongoModel();

  void setQuery(const mongo::Query &query);

  QModelIndex parent(const QModelIndex & index) const;
  int rowCount(const QModelIndex & parent = QModelIndex()) const;
  int columnCount(const QModelIndex & parent = QModelIndex()) const;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const;
  QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
  bool setData(const QModelIndex & index, const QVariant & value,
               int role = Qt::EditRole);
  Qt::ItemFlags flags(const QModelIndex & index) const;

  QModelIndex index(int row, int column,
                    const QModelIndex & parent = QModelIndex()) const;

  void clear();

  /** Set the 2D image for the molecule at row. */
  bool setImage2D(int row, const QByteArray &image);

private:
  class Private;
  Private *d;

private slots:
};

} // End namespace

#endif
