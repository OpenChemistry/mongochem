/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2013 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef MONGOCHEM_QUERYPROGRESSDIALOG_H
#define MONGOCHEM_QUERYPROGRESSDIALOG_H

#include <QDialog>

namespace Ui {
class QueryProgressDialog;
}

namespace MongoChem {

/**
 * @class QueryProgressDialog
 *
 * The QueryProgressDialog provides a simple dialog for displaying the
 * progress of a query. Also contains a cancel button so that the user
 * can terminate the query.
 */
class QueryProgressDialog : public QDialog
{
  Q_OBJECT

public:
  /**
   * Creates a new query progress dialog.
   */
  explicit QueryProgressDialog(QWidget *parent_ = 0);

  /**
   * Destroys the query progress dialog.
   */
  ~QueryProgressDialog();

  /**
   * Set the current value to @p value_.
   */
  void setValue(int value_);

  /**
   * Returns the current value.
   */
  int value() const;

  /**
   * Returns true if the user clicked the cancel button.
   */
  bool wasCanceled() const;

private slots:
  void cancelClicked();

private:
  Q_DISABLE_COPY(QueryProgressDialog)

  Ui::QueryProgressDialog *ui;

  int m_value;
  bool m_wasCancelled;
};

} // end MongoChem namespace

#endif // MONGOCHEM_QUERYPROGRESSDIALOG_H
