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

#ifndef RPCLISTENER_H
#define RPCLISTENER_H

#include <chemdata/rpc/export.h>

#include <vector>

#include <QObject>

#include <molequeue/transport/connection.h>
#include <molequeue/transport/connectionlistener.h>

namespace ChemData {

/// The RpcListener listens for ChemData RPC calls and executes the
/// corresponding methods.
class CHEMDATARPC_EXPORT RpcListener : public QObject
{
  Q_OBJECT

public:
  explicit RpcListener(QObject *parent = 0);
  ~RpcListener();

  void start();

private slots:
  void connectionEstablished(MoleQueue::Connection *);
  void connectionDisconnected();
  void connectionError(MoleQueue::ConnectionListener::Error, const QString &);
  void messageReceived(const MoleQueue::Message message);

private:
  std::vector<MoleQueue::Connection *> m_connections;
  MoleQueue::ConnectionListener *m_connectionListener;
};

} // end ChemData namespace

#endif // RPCLISTENER_H
