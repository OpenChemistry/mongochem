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

#ifndef OPENINEDITORHANDLER_H
#define OPENINEDITORHANDLER_H

#include <QObject>
#include <qjsonobject.h>

#include <chemkit/molecule.h>

#include "moleculeref.h"

namespace MoleQueue {
class JsonRpcClient;
}

namespace MongoChem {

class OpenInEditorHandler : public QObject
{
  Q_OBJECT

public:
  /** Creates a new open in editor handler. */
  explicit OpenInEditorHandler(QObject *parent = 0);

  /** Destroys the open in editor handler object. */
  ~OpenInEditorHandler();

  /** Sets the name of the editor application to use. */
  void setEditor(const QString &name);

  /** Returns the named of the editor application. */
  QString editor() const;

  /** Sets the molecule to @p molecule. */
  void setMolecule(const MoleculeRef &molecule);

  /** Returns the molecule. */
  MoleculeRef molecule() const;

public slots:
  /**
   * This slot is called when the molecule should be opened in a molecular
   * editor.
   */
  void openInEditor();

private slots:
  void rpcResultReceived(QJsonObject object);
  void rpcErrorReceived(QJsonObject object);

private:
  QString m_editorName;
  MoleculeRef m_moleculeRef;
  MoleQueue::JsonRpcClient *m_rpcClient;
};

} // end MongoChem namespace

#endif // OPENINEDITORHANDLER_H
