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

#include "rpclistener.h"

#include <QCoreApplication>

#include <mongochem/gui/mongodatabase.h>

#include "qjsonvalue.h"
#include "qjsonobject.h"

#include <molequeue/transport/jsonrpc.h>
#include <molequeue/transport/localsocketconnectionlistener.h>

namespace MongoChem {

RpcListener::RpcListener(QObject *parent_)
  : QObject(parent_)
{
  m_rpc = new MoleQueue::JsonRpc(this);

  m_connectionListener =
    new MoleQueue::LocalSocketConnectionListener(this, "mongochem");

  m_rpc->addConnectionListener(m_connectionListener);

  connect(m_rpc, SIGNAL(messageReceived(const MoleQueue::Message&)),
          this, SLOT(messageReceived(const MoleQueue::Message&)));
}

RpcListener::~RpcListener()
{
  m_rpc->removeConnectionListener(m_connectionListener);
  m_connectionListener->stop();
}

void RpcListener::start()
{
  m_connectionListener->start();
}

void RpcListener::connectionError(MoleQueue::ConnectionListener::Error error,
                                  const QString &message)
{
  Q_UNUSED(error)
  qDebug() << "RpcListener connection error: " << message;
}

void RpcListener::messageReceived(const MoleQueue::Message &message)
{
  MongoDatabase *db = MongoDatabase::instance();

  QString method = message.method();
  QJsonObject params = message.params().toObject();

  qDebug() << "got method: " << method;

  if (method == "getChemicalJson") {
    std::string inchi = params["inchi"].toString().toStdString();

    MongoChem::MoleculeRef molecule = db->findMoleculeFromInChI(inchi);
    if (molecule.isValid()) {
      mongo::BSONObj obj = db->fetchMolecule(molecule);

      std::string name = obj.getStringField("name");

      MoleQueue::Message response = message.generateResponse();
      response.setResult(QString::fromStdString(name));
      response.send();
    }
    else {
      // error
      MoleQueue::Message response = message.generateErrorResponse();
      response.setErrorCode(-1);
      response.setErrorMessage("Invalid Molecule Identifier");
      response.send();
    }
  }
  else if (method == "convertMoleculeIdentifier") {
    std::string identifier = params["identifier"].toString().toStdString();
    std::string input_format = params["inputFormat"].toString().toStdString();
    std::string output_format =
      params["outputFormat"].toString().toStdString();

    MongoChem::MoleculeRef molecule =
      db->findMoleculeFromIdentifier(identifier, input_format);

    if (!molecule.isValid()) {
      MoleQueue::Message response = message.generateErrorResponse();
      response.setErrorCode(-1);
      response.setErrorMessage("Invalid Molecule Identifier");
      response.send();
      return;
    }

    mongo::BSONObj obj = db->fetchMolecule(molecule);

    std::string result = obj.getStringField(output_format.c_str());

    MoleQueue::Message response = message.generateResponse();
    response.setResult(QString::fromStdString(result));
    response.send();
  }
  else if (method == "kill") {
    // Only allow MongoChem to be killed through RPC if it was started with the
    // '--testing' flag.
    if (qApp->arguments().contains("--testing"))
      qApp->quit();
    else
      qDebug() << "Ignoring kill command. Start with '--testing' to enable.";
  }
  else {
    MoleQueue::Message errorMessage = message.generateErrorResponse();
    errorMessage.setErrorCode(-32601);
    errorMessage.setErrorMessage("Method not found");
    QJsonObject errorDataObject;
    errorDataObject.insert("request", message.toJsonObject());
    errorMessage.setErrorData(errorDataObject);
    errorMessage.send();
  }
}

} // end MongoChem namespace
