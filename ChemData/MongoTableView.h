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

#ifndef MongoTableView_H
#define MongoTableView_H

#include <QtGui/QTableView>

class QNetworkAccessManager;
class QNetworkReply;

namespace ChemData
{

class MongoTableView : public QTableView
{
  Q_OBJECT

public:
  MongoTableView(QWidget *parent = 0);

  /** Custom context menu for this view. */
  void contextMenuEvent(QContextMenuEvent *e);

protected slots:
  /** This slot is fired when the job results should be opened in Avogadro. */
  void openInAvogadro();

  void fetchByCas();

  /** Retrieving structures from remote databases! */
  void replyFinished(QNetworkReply*);

protected:
  /** Used for fetching data from web services. */
  QNetworkAccessManager *m_network;
  QString m_moleculeName;
};

} // End of namespace

#endif
