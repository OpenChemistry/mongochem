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
  MoleQueue::JsonRpcClient rpcClient;
  rpcClient.connectToServer("avogadro");
  if (!rpcClient.isConnected()) {
    QMessageBox::critical(qobject_cast<QWidget *>(parent()),
                          tr("Connection Error"),
                          tr("Failed to connect to Avogadro"));
    return;
  }

  // load chemical json for the molecule
  MongoDatabase *db = MongoDatabase::instance();
  mongo::BSONObj moleculeObj = db->fetchMolecule(m_moleculeRef);
  std::string cjson = CjsonExporter::toCjson(moleculeObj);

  // create json rpc method
  QJsonObject request(rpcClient.emptyRequest());
  request["method"] = QLatin1String("loadMolecule");
  QJsonObject params;
  params["content"] = QLatin1String(cjson.c_str());
  params["format"] = QLatin1String("cjson");
  request["params"] = params;

  // send request
  rpcClient.sendRequest(request);
}

} // end MongoChem namespace
