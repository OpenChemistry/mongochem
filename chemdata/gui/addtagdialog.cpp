/******************************************************************************

  This source file is part of the ChemData project.

  Copyright 2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "addtagdialog.h"
#include "ui_addtagdialog.h"

#include <chemdata/core/mongodatabase.h>

namespace ChemData {

AddTagDialog::AddTagDialog(QWidget *parent_)
  : QDialog(parent_),
    ui(new Ui::AddTagDialog)
{
  ui->setupUi(this);

  m_completer = 0;
}

AddTagDialog::~AddTagDialog()
{
  delete ui;
}

void AddTagDialog::setCollection(const std::string &collection)
{
  m_collection = collection;

  // remove previous completer
  if (m_completer != 0) {
    ui->tagLineEdit->setCompleter(0);
    m_completer->deleteLater();
  }

  // setup auto-completer for line edit
  MongoDatabase *db = MongoDatabase::instance();
  std::vector<std::string> tags =
    db->fetchTagsWithPrefix(m_collection, "");
  QStringList tagList;
  foreach (const std::string &tag, tags)
    tagList.append(QString::fromStdString(tag));
  m_completer = new QCompleter(tagList, this);
  ui->tagLineEdit->setCompleter(m_completer);
}

std::string AddTagDialog::getTag(const std::string &collection,
                                 QWidget *parent_)
{
  // create dialog
  AddTagDialog dialog(parent_);

  // set collection
  dialog.setCollection(collection);

  // show the dialog
  int exitCode = dialog.exec();

  // return the tag or an empty string based on the users choice
  if (exitCode == QDialog::Accepted)
    return dialog.ui->tagLineEdit->text().toStdString();
  else
    return std::string();
}

} // end ChemData namespace
