/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2011-2013 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef EXPORTMOLECULEHANDLER_H
#define EXPORTMOLECULEHANDLER_H

#include <QtCore/QObject>

#include "moleculeref.h"

namespace MongoChem {

class ExportMoleculeHandler : public QObject
{
  Q_OBJECT

public:
  ExportMoleculeHandler(QObject *parent = 0);
  ~ExportMoleculeHandler();

  void setMolecule(const MoleculeRef &molecule);

public slots:
  void exportMolecule();

private:
  MoleculeRef m_molecule;
};

} // end MongoChem namespace

#endif // EXPORTMOLECULEHANDLER_H
