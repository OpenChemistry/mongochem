/******************************************************************************

  This source file is part of the ChemData project.

  Copyright 2011-2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "substructurefiltermodel.h"

#include <boost/make_shared.hpp>

#include <mongo/client/dbclient.h>

#include <chemkit/molecule.h>

using namespace mongo;

SubstructureFilterModel::SubstructureFilterModel(QObject *parent)
  : QSortFilterProxyModel(parent)
{
}

SubstructureFilterModel::~SubstructureFilterModel()
{
}

void SubstructureFilterModel::setSmiles(const QString &smiles)
{
  m_smiles = smiles;
  m_query.setMolecule(smiles.toStdString(), "smiles");
}

QString SubstructureFilterModel::smiles() const
{
  return m_smiles;
}

bool SubstructureFilterModel::filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const
{
  Q_UNUSED(sourceColumn);
  Q_UNUSED(sourceParent);

  // accept and display all columns
  return true;
}

bool SubstructureFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
  if(!m_query.molecule() || m_query.molecule()->isEmpty()){
    return false;
  }

  // get the bson object for the row
  QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
  BSONObj *obj = static_cast<BSONObj *>(index.internalPointer());
  if(!obj){
    return false;
  }

  // get the molecule's inchi
  BSONElement elem = obj->getField("inchi");
  if(elem.eoo()){
    return false;
  }

  // create a molecule from the inchi formula
  chemkit::Molecule molecule(elem.str(), "inchi");

  // test the molecule against the query structure
  return m_query.matches(&molecule);
}
