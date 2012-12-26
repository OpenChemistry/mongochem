/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "computationalresultstableview.h"

#include <QMenu>
#include <QTextEdit>
#include <QHeaderView>
#include <QMessageBox>
#include <QContextMenuEvent>

#include <mongo/client/dbclient.h>
#include <mongo/client/gridfs.h>

#include "mongodatabase.h"
#include "openineditorhandler.h"
#include "moleculedetaildialog.h"

namespace MongoChem {

ComputationalResultsTableView::ComputationalResultsTableView(QWidget *parent_)
  : QTableView(parent_),
    m_enableShowDetailsAction(false),
    m_openInEditorHandler(new OpenInEditorHandler(this))
{
  setAlternatingRowColors(true);
  setSelectionBehavior(QAbstractItemView::SelectRows);
}

ComputationalResultsTableView::~ComputationalResultsTableView()
{
}

void ComputationalResultsTableView::contextMenuEvent(QContextMenuEvent *e)
{
  MongoDatabase *db = MongoDatabase::instance();
  QModelIndex index = indexAt(e->pos());

  if (index.isValid()) {
    mongo::BSONObj *obj =
      static_cast<mongo::BSONObj *>(index.internalPointer());

    QMenu *menu = new QMenu(this);

    QAction *action;

    // add view and save log file actions
    if (obj && obj->hasField("log_file")) {
      action = menu->addAction("&View Log File",
                               this,
                               SLOT(showLogFile()));

      action = menu->addAction("&Save Log File",
                               this,
                               SLOT(showLogFile()));
    }

    // add open in editor action
    action = menu->addAction("&Open in Editor");
    MoleculeRef ref = db->findMoleculeFromBSONObj(obj);
    m_openInEditorHandler->setMolecule(ref);
    connect(action, SIGNAL(triggered()),
            m_openInEditorHandler, SLOT(openInEditor()));

    // add details action
    if (m_enableShowDetailsAction) {
      action = menu->addAction("Show Molecule &Details",
                               this,
                               SLOT(showMoleculeDetailsDialog()));
    }

    m_row = index.row();

    menu->exec(QCursor::pos());
  }
}

void ComputationalResultsTableView::showLogFile()
{
  MongoDatabase *db = MongoDatabase::instance();
  mongo::GridFS fs(*db->connection(), "chem", "fs");

  mongo::BSONObj *obj =
    static_cast<mongo::BSONObj *>(currentIndex().internalPointer());
  if (obj) {
    const char *file_name = obj->getStringField("log_file");

    mongo::GridFile file = fs.findFile(file_name);

    if (!file.exists()) {
      QMessageBox::critical(this,
                            "Error",
                            "Failed to load output file.");
      return;
    }

    // load file into a buffer
    std::stringstream stream;
    file.write(stream);

    QTextEdit *viewer = new QTextEdit(this);
    viewer->resize(500, 600);
    viewer->setWindowFlags(Qt::Dialog);
    viewer->setWindowTitle(file_name);
    viewer->setReadOnly(true);
    std::string file_data_string = stream.str();
    viewer->setText(file_data_string.c_str());
    viewer->show();
  }
  else {
    QMessageBox::critical(this,
                          "Error",
                          "Failed to load output file.");
  }
}

void ComputationalResultsTableView::showMoleculeDetailsDialog()
{
  MongoDatabase *db = MongoDatabase::instance();
  mongo::BSONObj *obj =
    static_cast<mongo::BSONObj *>(currentIndex().internalPointer());
  std::string inchikey = obj->getStringField("inchikey");
  MoleculeRef ref = db->findMoleculeFromInChIKey(inchikey);

  if (ref.isValid()) {
    MoleculeDetailDialog *dialog = new MoleculeDetailDialog(this);
    dialog->setMolecule(ref);
    dialog->show();
  }
  else {
    QMessageBox::critical(this,
                          "Error",
                          "Failed to find molecule in collection.");
  }
}

} // end MongoChem namespace
