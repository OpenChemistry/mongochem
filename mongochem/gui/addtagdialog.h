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

#ifndef ADDTAGDIALOG_H
#define ADDTAGDIALOG_H

#include <string>

#include <QDialog>
#include <QCompleter>

namespace Ui {
class AddTagDialog;
}

namespace MongoChem {

/**
 * Simple dialog allowing the user to specify a new tag. Will auto-complete
 * with existing tags as the user types.
 */
class AddTagDialog : public QDialog
{
  Q_OBJECT

public:
  explicit AddTagDialog(QWidget *parent = 0);
  ~AddTagDialog();

  /** Sets the collection to use for auto-completion. */
  void setCollection(const std::string &collection);

  /**
   * Prompts the user for a new tag. Uses the current tags from @p collection
   * to auto-complete as the user types. Returns the tag if the user accepts
   * or an empty string if the user cancels.
   */
  static std::string getTag(const std::string &collection,
                            QWidget *parent_ = 0);

private:
  Ui::AddTagDialog *ui;
  QCompleter *m_completer;
  std::string m_collection;
};

} // end MongoChem namespace

#endif // ADDTAGDIALOG_H
