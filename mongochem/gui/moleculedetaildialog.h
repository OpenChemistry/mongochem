/******************************************************************************

  This source file is part of the MongoChem project.

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

#include "moleculeref.h"

namespace Avogadro {
namespace QtGui {
class ScenePlugin;
class Molecule;
class ToolPluginFactory;
class ToolPlugin;
}
}

namespace Ui {
class MoleculeDetailDialog;
}

class QTableWidgetItem;

namespace MongoChem {

class MoleculeRef;
class OpenInEditorHandler;
class ExportMoleculeHandler;
class ComputationalResultsModel;
class ComputationalResultsTableView;

class MoleculeDetailDialog : public QDialog
{
  Q_OBJECT

public:
  explicit MoleculeDetailDialog(QWidget *parent = 0);
  ~MoleculeDetailDialog();

  void setMolecule(const MoleculeRef &molecule);
  bool setMoleculeFromInchi(const std::string &inchi);

private slots:
  void addNewAnnotation();
  void annotationRightClicked(const QPoint & pos);
  void editCurrentAnnotation();
  void deleteCurrentAnnotation();
  void reloadAnnotations();
  void annotationItemChanged(QTableWidgetItem *item);
  void reloadTags();
  void addNewTag();
  void removeTag(const QString &tag);
  void removeSelectedTag();
  void tagsRightClicked(const QPoint &pos);

private:
  MoleculeRef m_ref;
  Ui::MoleculeDetailDialog *ui;
  ExportMoleculeHandler *m_exportHandler;
  OpenInEditorHandler *m_openInEditorHandler;
  ComputationalResultsModel *m_computationalResultsModel;
  ComputationalResultsTableView *m_computationalResultsTableView;
  Avogadro::QtGui::ScenePlugin *m_scenePlugin;
  Avogadro::QtGui::ToolPlugin *m_toolPlugin;
  Avogadro::QtGui::Molecule *m_molecule;

  void updateScenePlugins();
  void updateToolPlugins();
};

} // end MongoChem namespace

#endif // MOLECULEDETAILDIALOG_H
