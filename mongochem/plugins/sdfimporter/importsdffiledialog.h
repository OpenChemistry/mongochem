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

#ifndef MONGOCHEM_IMPORTSDFFILEDIALOG_H
#define MONGOCHEM_IMPORTSDFFILEDIALOG_H

#include <mongochem/gui/abstractimportdialog.h>

#include <QtCore/QSet>

class QProgressDialog;

namespace Ui {
class ImportSdfFileDialog;
}

namespace MongoChem {
class SvgGenerator;
}

/**
 * The ImportSdfFileDialog class provides a dialog allowing for the user
 * to import molecular data from a SDF file.
 */
class ImportSdfFileDialog : public MongoChem::AbstractImportDialog
{
  Q_OBJECT

public:
  explicit ImportSdfFileDialog(QWidget *parent_ = 0);
  ~ImportSdfFileDialog();

  void setFileName(const QString &fileName_);
  QString fileName() const;

public slots:
  void openFile();

private slots:
  void import();
  void moleculeDiagramReady(int errorCode);

private:
  Ui::ImportSdfFileDialog *ui;
  QString m_fileName;
  QSet<MongoChem::SvgGenerator *> m_svgGenerators;
  QProgressDialog *m_progressDialog;
};

#endif // MONGOCHEM_IMPORTSDFFILEDIALOG_H
