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

#ifndef MONGOCHEM_RPCLISTENER_H
#define MONGOCHEM_RPCLISTENER_H

#include <QObject>

#include <molequeue/servercore/connectionlistener.h>

#include <qjsonobject.h>

namespace MoleQueue {
class JsonRpc;
class JsonRpcClient;
class Message;
}

namespace MongoChem {

/**
 * The RpcListener listens for MongoChem RPC calls and executes the
 * corresponding methods.
 */
class RpcListener : public QObject
{
  Q_OBJECT

public:
  explicit RpcListener(QObject *parent = 0);
  ~RpcListener();

  void start();

signals:
  void showSimilarMolecules(const std::string &identifier,
                            const std::string &format);

private slots:
  void connectionError(MoleQueue::ConnectionListener::Error, const QString &);
  void receivePingResponse(const QJsonObject &response = QJsonObject());
  void messageReceived(const MoleQueue::Message &message);

private:
  MoleQueue::JsonRpc *m_rpc;
  MoleQueue::ConnectionListener *m_connectionListener;
  MoleQueue::JsonRpcClient *m_pingClient;
};

} // end MongoChem namespace

#endif // MONGOCHEM_RPCLISTENER_H
