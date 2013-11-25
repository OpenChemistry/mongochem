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

#include "openineditorhandler.h"

#include <QMessageBox>

#include <molequeue/client/jsonrpcclient.h>

#include "cjsonexporter.h"
#include "mongodatabase.h"

namespace MongoChem {

using std::string;

OpenInEditorHandler::OpenInEditorHandler(QObject *parent_) :
    QObject(parent_)
{
  // by default use the avogadro editor
  m_editorName = "avogadro";

  m_rpcClient = new MoleQueue::JsonRpcClient(this);
}

OpenInEditorHandler::~OpenInEditorHandler()
{
  delete m_rpcClient;
}

void OpenInEditorHandler::setEditor(const QString &name)
{
  m_editorName = name;
}

QString OpenInEditorHandler::editor() const
{
  return m_editorName;
}

void OpenInEditorHandler::setMolecule(const MoleculeRef &molecule_)
{
  m_moleculeRef = molecule_;
}

MoleculeRef OpenInEditorHandler::molecule() const
{
  return m_moleculeRef;
}

void OpenInEditorHandler::openInEditor()
{
  if (!m_moleculeRef.isValid())
    return;

  // Load Chemical JSON for the molecule if possible.
  MongoDatabase *db = MongoDatabase::instance();
  mongo::BSONObj moleculeObj = db->fetchMolecule(m_moleculeRef);

  // Create a JSON-RPC request.
  QJsonObject request(m_rpcClient->emptyRequest());
  request["method"] = QLatin1String("loadMolecule");

  // Check if the molecule has 3D geometry.
  if (moleculeObj.hasField("3dStructure")) {
    // Generate a Chemical JSON file and use that.
    string cjson = CjsonExporter::toCjson(moleculeObj);

    // Create the JSON-RPC method call.
    QJsonObject params;
    params["content"] = QLatin1String(cjson.c_str());
    params["format"] = QLatin1String("cjson");
    request["params"] = params;
  }
  else if (moleculeObj.hasField("inchi")) {
    // Create a JSON-RPC method call using the InChI.
    QJsonObject params;
    params["content"] = "InChI="
        + QLatin1String(moleculeObj.getStringField("inchi"));
    params["format"] = QLatin1String("inchi");
    request["params"] = params;
  }
  else {
    QMessageBox::critical(qobject_cast<QWidget *>(parent()),
                          tr("Incomplete Molecule"),
                          tr("Error: Molecule does not have 3D coordinates"));
    return;
  }

  // connect to avogadro
  if (!m_rpcClient->isConnected()) {
    m_rpcClient->connectToServer("avogadro");
    if (!m_rpcClient->isConnected()) {
      QMessageBox::critical(qobject_cast<QWidget *>(parent()),
                            tr("Connection Error"),
                            tr("Failed to connect to Avogadro"));
      return;
    }
  }

  // connect to signals
  connect(m_rpcClient, SIGNAL(resultReceived(QJsonObject)),
          this, SLOT(rpcResultReceived(QJsonObject)));
  connect(m_rpcClient, SIGNAL(errorReceived(QJsonObject)),
          this, SLOT(rpcErrorReceived(QJsonObject)));

  // send request
  m_rpcClient->sendRequest(request);
}

void OpenInEditorHandler::rpcResultReceived(QJsonObject object)
{
  Q_UNUSED(object);
}

void OpenInEditorHandler::rpcErrorReceived(QJsonObject object)
{
  // show dialog with error message
  QString error = object["message"].toString();
  QMessageBox::critical(qobject_cast<QWidget *>(parent()),
                        tr("RPC Error"),
                        tr("Failed to open molecule: %1").arg(error));
}

} // end MongoChem namespace
