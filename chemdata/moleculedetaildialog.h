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

#include <mongo/client/dbclient.h>

#include <QDialog>

namespace Ui {
class MoleculeDetailDialog;
}

class MoleculeRef;
class OpenInEditorHandler;
class ExportMoleculeHandler;

class MoleculeDetailDialog : public QDialog
{
  Q_OBJECT

public:
  explicit MoleculeDetailDialog(QWidget *parent = 0);
  ~MoleculeDetailDialog();

  void setMolecule(const MoleculeRef &molecule);
  bool setMoleculeFromInchi(const std::string &inchi);

private:
  Ui::MoleculeDetailDialog *ui;
  ExportMoleculeHandler *m_exportHandler;
  OpenInEditorHandler *m_openInEditorHandler;
};

#endif // MOLECULEDETAILDIALOG_H
