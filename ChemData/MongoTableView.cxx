/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2011 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "MongoTableView.h"

#include "mongomodel.h"

#include <mongo/client/dbclient.h>
#include <mongo/client/gridfs.h>

#include <QtGui/QMenu>
#include <QtGui/QContextMenuEvent>
#include <QtGui/QMessageBox>
#include <QtCore/QProcess>
#include <QtCore/QFile>
#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>

namespace ChemData
{

MongoTableView::MongoTableView(QWidget *parent) : QTableView(parent),
  m_network(0), m_row(-1)
{
}

void MongoTableView::contextMenuEvent(QContextMenuEvent *e)
{
  QModelIndex index = indexAt(e->pos());
  if (index.isValid()) {
    m_row = index.row();
    mongo::BSONObj *obj = static_cast<mongo::BSONObj *>(index.internalPointer());
    QMenu *menu = new QMenu(this);

    QAction *action;
    mongo::BSONElement inchi = obj->getField("InChI");
    mongo::BSONElement iupac = obj->getField("IUPAC");
    mongo::BSONElement cml = obj->getField("CML File");

    if (inchi.eoo()) {
      action = menu->addAction("&Fetch chemical structure");
      action->setData(obj->getField("CAS").str().c_str());
      connect(action, SIGNAL(triggered()), this, SLOT(fetchByCas()));
    }
    else {
      action = menu->addAction("&Refetch chemical structure");
      action->setData(obj->getField("CAS").str().c_str());
      connect(action, SIGNAL(triggered()), this, SLOT(fetchByCas()));
    }

    if (!inchi.eoo() && iupac.eoo()) {
      // The field exists, there is more we can do here!
      action = menu->addAction("&Fetch IUPAC name");
      action->setData(inchi.str().c_str());
      connect(action, SIGNAL(triggered()), this, SLOT(fetchIUPAC()));
    }

    if (!cml.eoo()) {
      action = menu->addAction("&Open in Avogadro");
      action->setData(cml.str().c_str());
      m_moleculeName = QString(obj->getField("CAS").str().c_str()) + ".cml";
      connect(action, SIGNAL(triggered()), this, SLOT(openInAvogadro()));
    }

    menu->exec(QCursor::pos());
  }
}

void MongoTableView::openInAvogadro()
{
  QAction *action = static_cast<QAction*>(sender());
  if (action) {
    QFile file(m_moleculeName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
      return;
    file.write(action->data().toByteArray());
    file.close();
    QProcess::startDetached("/home/marcus/ssd/build/avogadro-squared/prefix/bin/avogadro " + m_moleculeName);
  }
}

void MongoTableView::fetchByCas()
{
  if (!m_network) {
    m_network = new QNetworkAccessManager(this);
    connect(m_network, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
  }
  QAction *action = static_cast<QAction*>(sender());
  // Prompt for a chemical structure name
  QString structureName = action->data().toString();
  if (structureName.isEmpty())
    return;
  else
    while (structureName[0] == '0')
      structureName.remove(0, 1);
  // Fetch the CAS from the NIH database
  m_network->get(QNetworkRequest(
      QUrl("http://cactus.nci.nih.gov/chemical/structure/" + structureName + "/sdf?get3d=true")));

  m_moleculeName = structureName + ".sdf";
  qDebug() << "structure to fetch:" << m_moleculeName;
}

void MongoTableView::fetchIUPAC()
{
  if (!m_network) {
    m_network = new QNetworkAccessManager(this);
    connect(m_network, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
  }
  QAction *action = static_cast<QAction*>(sender());
  // Prompt for a chemical structure name
  QString inchi = action->data().toString();
  if (inchi.isEmpty())
    return;

  // Fetch the IUPAC name from the NIH database
  QString url = "http://cactus.nci.nih.gov/chemical/structure/" + inchi
      + "/iupac_name";
  m_network->get(QNetworkRequest(QUrl(url)));
  qDebug() << url;

  m_moleculeName = "IUPAC";
}

void MongoTableView::replyFinished(QNetworkReply *reply)
{
  // Read in all the data
  if (!reply->isReadable()) {
    QMessageBox::warning(qobject_cast<QWidget*>(parent()),
                         tr("Network Download Failed"),
                         tr("Network timeout or other error."));
    reply->deleteLater();
    return;
  }

  QByteArray data = reply->readAll();
  MongoModel *model = qobject_cast<MongoModel *>(this->model());

  // Check if the structure was successfully downloaded
  if (data.contains("Page not found (404)")) {
    QMessageBox::warning(qobject_cast<QWidget*>(parent()),
                         tr("Network Download Failed"),
                         tr("Specified molecule could not be found: %1").arg(m_moleculeName));
    reply->deleteLater();
    return;
  }
  qDebug() << "Structure:" << data;
  QFile file(m_moleculeName);
  if (m_moleculeName == "IUPAC") {
    qDebug() << "IUPAC Name:" << data;
    model->setIUPACName(m_row, data);
    return;
  }
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return;
  file.write(data);
  file.close();

  QProcess descriptors;
  descriptors.start(QCoreApplication::applicationDirPath() +
                    "/descriptors " + m_moleculeName);
  if (!descriptors.waitForFinished()) {
    qDebug() << "Failed to calculate descriptors.";
    return;
  }
  QByteArray result = descriptors.readAllStandardOutput();
  model->addIdentifiers(m_row, result);
}

} // End of namespace
