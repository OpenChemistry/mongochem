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

OpenInEditorHandler::OpenInEditorHandler(QObject *parent_) :
    QObject(parent_)
{
  // by default use the avogadro editor
  m_editorName = "avogadro";
}

OpenInEditorHandler::~OpenInEditorHandler()
{
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

  // connect to avogadro
  MoleQueue::JsonRpcClient *rpcClient = new MoleQueue::JsonRpcClient(this);
  rpcClient->connectToServer("avogadro");
  if (!rpcClient->isConnected()) {
    QMessageBox::critical(qobject_cast<QWidget *>(parent()),
                          tr("Connection Error"),
                          tr("Failed to connect to Avogadro"));
    delete rpcClient;
    return;
  }

  // load chemical json for the molecule
  MongoDatabase *db = MongoDatabase::instance();
  mongo::BSONObj moleculeObj = db->fetchMolecule(m_moleculeRef);
  std::string cjson = CjsonExporter::toCjson(moleculeObj);

  // create json rpc method
  QJsonObject request(rpcClient->emptyRequest());
  QJsonValue id = request["id"];
  request["method"] = QLatin1String("loadMolecule");
  QJsonObject params;
  params["content"] = QLatin1String(cjson.c_str());
  params["format"] = QLatin1String("cjson");
  request["params"] = params;

  // connect to signals
  connect(rpcClient, SIGNAL(resultReceived(QJsonObject)),
          this, SLOT(rpcResultReceived(QJsonObject)));
  connect(rpcClient, SIGNAL(errorReceived(QJsonObject)),
          this, SLOT(rpcErrorReceived(QJsonObject)));

  // send request
  rpcClient->sendRequest(request);
}

void OpenInEditorHandler::rpcResultReceived(QJsonObject object)
{
  MoleQueue::JsonRpcClient *client =
    qobject_cast<MoleQueue::JsonRpcClient *>(sender());
  if (!client)
    return;

  // destroy the client object
  client->deleteLater();
}

void OpenInEditorHandler::rpcErrorReceived(QJsonObject object)
{
  MoleQueue::JsonRpcClient *client =
    qobject_cast<MoleQueue::JsonRpcClient *>(sender());
  if (!client)
    return;

  // show dialog with error message
  QString error = object["message"].toString();
  QMessageBox::critical(qobject_cast<QWidget *>(parent()),
                        tr("RPC Error"),
                        tr("Failed to open molecule: %1").arg(error));

  // destroy the client object
  client->deleteLater();
}

} // end MongoChem namespace
