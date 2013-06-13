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

#include <QtGui/QApplication>
#include <QtGui/QInputDialog>

#include <QtCore/QTimer>

#include <QtNetwork/QLocalServer>

#include <mongochem/gui/mongodatabase.h>

#include "qjsonvalue.h"
#include "qjsonobject.h"

#include <molequeue/client/jsonrpcclient.h>
#include <molequeue/servercore/jsonrpc.h>
#include <molequeue/servercore/localsocketconnectionlistener.h>

namespace MongoChem {

RpcListener::RpcListener(QObject *parent_)
  : QObject(parent_),
    m_pingClient(NULL)
{
  m_rpc = new MoleQueue::JsonRpc(this);

  m_connectionListener =
    new MoleQueue::LocalSocketConnectionListener(this, "mongochem");

  // when testing we forcibly remove any other mongochem rpc listeners
  // which may have be left over from other failed test runs
  if (qApp->arguments().contains("--testing"))
    QLocalServer::removeServer("mongochem");

  connect(m_connectionListener,
          SIGNAL(connectionError(MoleQueue::ConnectionListener::Error,QString)),
          SLOT(connectionError(MoleQueue::ConnectionListener::Error,QString)));

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
  qDebug() << "Error starting RPC server:" << message;
  if (error == MoleQueue::ConnectionListener::AddressInUseError) {
    // Try to ping the existing server to see if it is alive:
    if (!m_pingClient)
      m_pingClient = new MoleQueue::JsonRpcClient(this);
    bool result(m_pingClient->connectToServer(
                  m_connectionListener->connectionString()));

    if (result) {
      QJsonObject request(m_pingClient->emptyRequest());
      request["method"] = QLatin1String("internalPing");
      connect(m_pingClient, SIGNAL(resultReceived(QJsonObject)),
              SLOT(receivePingResponse(QJsonObject)));
      result = m_pingClient->sendRequest(request);
    }

    // If any of the above failed, trigger a failure now:
    if (!result)
      receivePingResponse();
    else // Otherwise wait 200 ms
      QTimer::singleShot(200, this, SLOT(receivePingResponse()));
  }
}

void RpcListener::receivePingResponse(const QJsonObject &response)
{
  // Disconnect and remove the ping client the first time this is called:
  if (m_pingClient) {
    m_pingClient->deleteLater();
    m_pingClient = NULL;
  }
  else {
    // In case the single shot timeout is triggered after the slot is called
    // directly or in response to m_pingClient's signal.
    return;
  }

  bool pingSuccessful = response.value("result").toString() == QString("pong");
  if (pingSuccessful) {
    qDebug() << "Other server is alive. Not starting new instance.";
  }
  else {
    QString title(tr("Error starting RPC server:"));
    QString label(
          tr("An error occurred while starting MongoChem's RPC listener. "
             "This may be happen for a\nnumber of reasons:\n\t"
             "A previous instance of MongoChem may have crashed.\n\t"
             "A running MongoChem instance was too busy to respond.\n\n"
             "If no other MongoChem instance is running on this machine, it "
             "is safe to replace the dead\nserver. "
             "Otherwise, this instance of MongoChem may be started without "
             "RPC capabilities\n(this will prevent RPC enabled applications "
             "from communicating with MongoChem)."));
    QStringList items;
    items << tr("Replace the dead server with a new instance.");
    items << tr("Start without RPC capabilities.");
    bool ok(false);
    QString item(QInputDialog::getItem(NULL, title, label, items, 0, false,
                                       &ok));

    if (ok && item == items.first()) {
      qDebug() << "Starting new server.";
      m_connectionListener->stop(true);
      m_connectionListener->start();
    }
    else {
      qDebug() << "Starting without RPC capabilities.";
    }
  }
}

void RpcListener::messageReceived(const MoleQueue::Message &message)
{
  MongoDatabase *db = MongoDatabase::instance();

  QString method = message.method();
  QJsonObject params = message.params().toObject();

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
  else if (method == "findSimilarMolecules") {
    std::string identifier = params["identifier"].toString().toStdString();
    std::string inputFormat = params["inputFormat"].toString().toStdString();

    emit showSimilarMolecules(identifier, inputFormat);

    MoleQueue::Message response = message.generateResponse();
    response.setResult(QString("ok"));
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
