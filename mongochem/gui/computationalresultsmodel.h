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

#ifndef MONGOCHEM_COMPUTATIONALRESULTSMODEL_H
#define MONGOCHEM_COMPUTATIONALRESULTSMODEL_H

#include <vector>

#include <QAbstractItemModel>

#include <mongo/client/dbclient.h>

namespace MongoChem {

/**
 * @class ComputationalResultsModel
 *
 * The ComputationalResultsModel class implements a Qt abstract item model for
 * accessing the computational job results in the database.
 */
class ComputationalResultsModel : public QAbstractItemModel
{
  Q_OBJECT

public:
  explicit ComputationalResultsModel(QObject *parent_ = 0);
  ~ComputationalResultsModel();

  /** Sets the Mongo query for the model to pull data from. */
  void setQuery(const mongo::Query &query);

  QModelIndex index(int row, int column, const QModelIndex &parent) const;
  QModelIndex parent(const QModelIndex &child) const;
  int rowCount(const QModelIndex &parent) const;
  int columnCount(const QModelIndex &parent) const;
  QVariant data(const QModelIndex &index, int role) const;
  QVariant headerData(int section,
                      Qt::Orientation orientation,
                      int role = Qt::DisplayRole) const;
  Qt::ItemFlags flags(const QModelIndex &index) const;

private:
  Q_DISABLE_COPY(ComputationalResultsModel)

  std::vector<mongo::BSONObj> m_objects;
};

} // end MongoChem namespace

#endif // MONGOCHEM_COMPUTATIONALRESULTSMODEL_H
