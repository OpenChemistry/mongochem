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

#include "rpclistener.h"

#include <QCoreApplication>

#include <sstream>

#include <chemdata/core/mongodatabase.h>

#include "../../thirdparty/jsoncpp/json/json.h"

#include <molequeue/transport/localsocketconnectionlistener.h>

namespace ChemData {

RpcListener::RpcListener(QObject *parent_)
  : QObject(parent_)
{
  m_connectionListener = new MoleQueue::LocalSocketConnectionListener(this, "chemdata");
  connect(m_connectionListener, SIGNAL(newConnection(MoleQueue::Connection*)),
          this, SLOT(connectionEstablished(MoleQueue::Connection*)));
  connect(m_connectionListener, SIGNAL(connectionError(MoleQueue::ConnectionListener::Error, const QString &)),
          this, SLOT(connectionError(MoleQueue::ConnectionListener::Error, const QString &)));
}

RpcListener::~RpcListener()
{
  m_connectionListener->stop();
}

void RpcListener::start()
{
  m_connectionListener->start();
}

void RpcListener::connectionEstablished(MoleQueue::Connection *connection)
{
  m_connections.push_back(connection);

  connect(connection, SIGNAL(newMessage(const MoleQueue::Message)),
          this, SLOT(messageReceived(const MoleQueue::Message)));
  connect(connection, SIGNAL(disconnected()),
          this, SLOT(connectionDisconnected()));

  connection->start();
}

void RpcListener::connectionDisconnected()
{
  MoleQueue::Connection *conn =
    qobject_cast<MoleQueue::Connection *>(this->sender());
  if (conn)
    conn->close();
}

void RpcListener::connectionError(MoleQueue::ConnectionListener::Error error,
                                  const QString &message)
{
  Q_UNUSED(error)

  qDebug() << "RpcListener connection error: " << message;
}

void RpcListener::messageReceived(const MoleQueue::Message message)
{
  MongoDatabase *db = MongoDatabase::instance();

  MoleQueue::Connection *conn =
    qobject_cast<MoleQueue::Connection *>(this->sender());

  Json::Value root;
  Json::Reader reader;

  bool ok = reader.parse(message.data().constData(), root);
  if (!ok) {
    Json::Value reply;
    reply["error"] = "Invalid JSON";
    conn->send(MoleQueue::Message(reply));
    return;
  }

  std::string method = root["method"].asString();
  std::cout << "method: " << method << std::endl;

  if (method == "get_chemical_json") {
    std::string inchi = root["params"]["inchi"].asString();

    ChemData::MoleculeRef molecule = db->findMoleculeFromInChI(inchi);
    if (molecule.isValid()) {
      mongo::BSONObj obj = db->fetchMolecule(molecule);

      Json::Value reply;
      reply["name"] = obj.getStringField("name");
      conn->send(MoleQueue::Message(reply));
    }
    else {
      // error
      Json::Value reply;
      reply["error"] = "Invalid Molecule Identifier";
      conn->send(MoleQueue::Message(reply));
    }
  }
  else if (method == "convert_molecule_identifier") {
    std::string identifier = root["params"]["identifier"].asString();
    std::string input_format = root["params"]["input_format"].asString();
    std::string output_format = root["params"]["output_format"].asString();

    ChemData::MoleculeRef molecule =
      db->findMoleculeFromIdentifer(identifier, input_format);

    if(!molecule.isValid()){
      Json::Value reply;
      reply["error"] = "Invalid Molecule Identifier";
      conn->send(MoleQueue::Message(reply));
      return;
    }

    mongo::BSONObj obj = db->fetchMolecule(molecule);

    Json::Value reply;
    reply[output_format] = obj.getStringField(output_format.c_str());
    conn->send(MoleQueue::Message(reply));
  }
  else if (method == "kill") {
    // only allow chemdata to be killed through rpc if it was
    // started with the '--testing' flag
    if (qApp->arguments().contains("--testing"))
      qApp->quit();
    else
      qDebug() << "Ignoring kill command. Start with '--testing' to enable.";
  }
  else {
    Json::Value reply;
    reply["error"] = "Invalid RPC Method";
    conn->send(MoleQueue::Message(reply));
  }
}

} // end ChemData namespace
