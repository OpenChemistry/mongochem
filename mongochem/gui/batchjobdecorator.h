/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2013 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef MONGOCHEM_BATCHJOBDECORATOR_H
#define MONGOCHEM_BATCHJOBDECORATOR_H

#include <avogadro/qtgui/batchjob.h>

#include "moleculeref.h"

namespace MongoChem {

class BatchJobDecorator : public Avogadro::QtGui::BatchJob
{
  Q_OBJECT
public:
  explicit BatchJobDecorator(QObject *parent = 0);
  explicit BatchJobDecorator(const QString &scriptFilePath,
                             QObject *parent = 0);
  ~BatchJobDecorator();

  void registerMoleculeRef(BatchId id, const MoleculeRef &mol);
  BatchId batchIdFromMoleculeRef(const MoleculeRef &mol) const;
  MoleculeRef moleculeRef(BatchId id) const;

private:
  QMap<BatchId, std::string> m_mongoIds;

};

inline void BatchJobDecorator::registerMoleculeRef(BatchJobDecorator::BatchId i,
                                                   const MoleculeRef &mol)
{
  m_mongoIds.insert(i, mol.id());
}

inline BatchJobDecorator::BatchId
BatchJobDecorator::batchIdFromMoleculeRef(const MoleculeRef &mol) const
{
  return m_mongoIds.key(mol.id());
}

inline MoleculeRef BatchJobDecorator::moleculeRef(BatchId id) const
{
  return MoleculeRef(m_mongoIds.value(id));
}

} // namespace MongoChem

#endif // MONGOCHEM_BATCHJOBDECORATOR_H
