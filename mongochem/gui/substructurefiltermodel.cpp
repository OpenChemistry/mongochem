/******************************************************************************

  This source file is part of the MongoChem project.

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

namespace MongoChem {

using namespace mongo;

SubstructureFilterModel::SubstructureFilterModel(QObject *parent_)
  : QSortFilterProxyModel(parent_)
{
}

SubstructureFilterModel::~SubstructureFilterModel()
{
}

void SubstructureFilterModel::setIdentifier(const QString &identifier_)
{
  m_identifier = identifier_;

  std::string format;
  if (identifier_.startsWith("InChI="))
    format = "inchi";
  else
    format = "smiles";
  m_query.setMolecule(identifier_.toStdString(), format);
}

QString SubstructureFilterModel::identifier() const
{
  return m_identifier;
}

bool SubstructureFilterModel::filterAcceptsColumn(int, const QModelIndex &) const
{
  // Accept and display all columns.
  return true;
}

bool SubstructureFilterModel::filterAcceptsRow(int sourceRow,
                                               const QModelIndex &sourceParent) const
{
  if (!m_query.molecule() || m_query.molecule()->isEmpty())
    return false;

  // get the bson object for the row
  QModelIndex index_ = sourceModel()->index(sourceRow, 0, sourceParent);
  BSONObj *obj = static_cast<BSONObj *>(index_.internalPointer());
  if (!obj)
    return false;

  // get the molecule's inchi
  BSONElement elem = obj->getField("inchi");
  if (elem.eoo())
    return false;

  // create a molecule from the inchi formula
  chemkit::Molecule molecule(elem.str(), "inchi");

  // test the molecule against the query structure
  return m_query.matches(&molecule);
}

} // end MongoChem namespace
