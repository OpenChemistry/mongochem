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

#include <QtCore/QSettings>
#include <QtGui/QApplication>

#include "mainwindow.h"

#ifdef MongoChem_ENABLE_RPC
# include <mongochem/gui/rpclistener.h>
#endif

int main(int argc, char *argv[])
{
  QCoreApplication::setOrganizationName("Kitware");
  QCoreApplication::setOrganizationDomain("kitware.com");
  QCoreApplication::setApplicationName("MongoChem");
  QCoreApplication::setApplicationVersion("0.1.0");
  QApplication app(argc, argv);

  // process command line options
  QString testFile;

  const QStringList &arguments = app.arguments();
  for (int i = 1; i < arguments.size(); i++) {
    const QString &argument = arguments[i];

    if (argument == "--test-file") {
      if (i + 1 < arguments.size()) {
        testFile = arguments[++i];
      }
      else {
        qWarning("--test-file option requires argument");
        return -1;
      }
    }
    else {
      qWarning("Unknown command line option: '%s'", qPrintable(argument));
      return -1;
    }
  }

  QSettings settings;
  if (!settings.contains("hostname"))
    settings.setValue("hostname", "localhost");
  if(!settings.contains("port"))
    settings.setValue("port", "27017");
  if(!settings.contains("collection"))
    settings.setValue("collection", "chem");
  if(!settings.contains("user"))
    settings.setValue("user", "unknown");

  // create main gui window
  MongoChem::MainWindow window;
  window.show();

  if (!testFile.isEmpty())
#ifdef QTTESTING
    window.playTestLater(testFile);
#else
    qWarning("MongoChem called with --test-file but testing is disabled.");
#endif

#ifdef MongoChem_ENABLE_RPC
  // create rpc listener
  MongoChem::RpcListener listener;
  listener.start();
#endif

  return app.exec();
}
