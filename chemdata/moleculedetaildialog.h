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

#ifndef MOLECULEDETAILDIALOG_H
#define MOLECULEDETAILDIALOG_H

#include <QDialog>

#include <mongo/client/dbclient.h>

namespace Ui {
class MoleculeDetailDialog;
}

class MoleculeDetailDialog : public QDialog
{
  Q_OBJECT

public:
  explicit MoleculeDetailDialog(QWidget *parent = 0);
  ~MoleculeDetailDialog();

  void setMoleculeObject(mongo::BSONObj *obj);

private:
  Ui::MoleculeDetailDialog *ui;
};

#endif // MOLECULEDETAILDIALOG_H
