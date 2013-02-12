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

class QueryProgressDialog : public QDialog
{
  Q_OBJECT

public:
  explicit QueryProgressDialog(QWidget *parent_ = 0);
  ~QueryProgressDialog();

  void setValue(int value_);
  int value() const;

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
