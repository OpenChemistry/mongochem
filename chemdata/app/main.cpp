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

#include <QtCore/QSettings>
#include <QtGui/QApplication>

#include <chemdata/gui/mainwindow.h>
#include <chemdata/rpc/rpclistener.h>

int main(int argc, char *argv[])
{
  QCoreApplication::setOrganizationName("Kitware");
  QCoreApplication::setOrganizationDomain("kitware.com");
  QCoreApplication::setApplicationName("ChemData");
  QCoreApplication::setApplicationVersion("0.1.0");
  QApplication app(argc, argv);

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
  ChemData::MainWindow window;
  window.show();

  // create rpc listener
  ChemData::RpcListener listener;
  listener.start();

  return app.exec();
}
