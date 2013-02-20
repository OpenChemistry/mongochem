#include <QtCore>
#include <QtDebug>

#include <molequeue/client/jsonrpcclient.h>

class Thread : public QThread
{
public:
  static void sleep(int sec) { return QThread::sleep(sec); }
};

class GetChemicalJsonTest : public QObject
{
  Q_OBJECT

public:
  GetChemicalJsonTest(QProcess *mongochem)
    : m_exitCode(-1),
      m_process(mongochem)
  {
    // connect to mongochem
    m_client.connectToServer("mongochem");

    connect(&m_client, SIGNAL(resultReceived(QJsonObject)),
            this, SLOT(gotResult(QJsonObject)));
    connect(&m_client, SIGNAL(errorReceived(QJsonObject)),
            this, SLOT(gotError(QJsonObject)));
  }

  bool isConnected() const
  {
    return m_client.isConnected();
  }

  void sendQuery()
  {
    QJsonObject request(m_client.emptyRequest());
    request["method"] = QLatin1String("getChemicalJson");

    QJsonObject params;
    params["inchi"] =  QLatin1String("InChI=1S/CH4O/c1-2/h2H,1H3");
    request["params"] = params;
    m_client.sendRequest(request);
  }

  void sendKill()
  {
    QJsonObject request(m_client.emptyRequest());
    request["method"] = QLatin1String("kill");
    m_client.sendRequest(request);
    m_client.flush();
    m_process->close();
    m_process->deleteLater();
    qApp->exit(m_exitCode);
  }

public slots:
  void gotResult(QJsonObject message)
  {
    QString result = message["result"].toString();
    if (result == "methanol")
      m_exitCode = 0;
    else
      m_exitCode = -2;

    sendKill();
  }

  void gotError(QJsonObject message)
  {
    qDebug() << "got error:" << message;

    m_exitCode = -1;

    sendKill();
  }

private:
  MoleQueue::JsonRpcClient m_client;
  int m_exitCode;
  QProcess *m_process;
};

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  QString executable = app.arguments()[1];
  qDebug() << "starting mongochem (" << executable << ")";

  QProcess *process = new QProcess;
  QStringList arguments;
  arguments += "--testing";
  process->start(executable, arguments);
  if (!process->waitForStarted()) {
    qWarning() << "Failed to start MongoChem: " << process->errorString();
    return -1;
  }

  Thread::sleep(2);

  GetChemicalJsonTest test(process);
  if (!test.isConnected()) {
    qWarning() << "Failed to connect to server";
    return -1;
  }

  test.sendQuery();

  return app.exec();
}

#include "get-chemical-json.moc"
