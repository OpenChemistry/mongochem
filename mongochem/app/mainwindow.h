/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2011 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtGui/QMainWindow>
#include <QtCore/QModelIndex>
#include <QtCore/QMap>

#include <vtkNew.h>
#include <vtkType.h>
#include <vtkAnnotationLink.h>
#include <vtkEventQtSlotConnect.h>

#include <mongochem/gui/moleculeref.h>

namespace mongo {
  class DBClientConnection;
}

namespace Ui {
  class MainWindow;
}

namespace MongoChem {
class AbstractVtkChartWidget;
class AbstractClusteringWidget;
class AbstractImportDialog;
}

namespace MongoChem {

class MongoModel;
class QuickQueryWidget;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow();
  ~MainWindow();

public slots:
  /** Queries the database for molecules that are similar to the molecule
    * specified by \p ref. Replaces the current selection with the \p count
    * most similar molecules.
    *
    * By default, this uses the tanimoto coefficent of the FP2 fingerprints
    * to determine similarity between molecules.
    */
  void showSimilarMolecules(MongoChem::MoleculeRef ref, size_t count = 10);

  /** Calculates and stores the fingerprint for each molecule in the
    * database.
    *
    * Returns \c false if the fingerprint specified by name is not valid.
    */
  bool calculateAndStoreFingerprints(const std::string &name = "fp2");

private:
  void setupTable();
  void setupCharts();
  void addMoleculesFromFile(const QString &fileName);

  /** Connects to MongoDB */
  void connectToDatabase();

  Ui::MainWindow *m_ui;
  mongo::DBClientConnection *m_db;
  MongoModel *m_model;
  QuickQueryWidget *m_queryWidget;
  vtkNew<vtkAnnotationLink> m_annotationLink;
  vtkNew<vtkEventQtSlotConnect> m_annotationEventConnector;

  QMap<QString, MongoChem::AbstractVtkChartWidget *> m_charts;
  QMap<QString, MongoChem::AbstractClusteringWidget *> m_clustering;
  QMap<QString, MongoChem::AbstractImportDialog *> m_importers;

private slots:
  /** Show the molecule details dialog for the molecule referred to
   *  by \p ref. */
  void showMoleculeDetailsDialog(MongoChem::MoleculeRef ref);

  /** Show the server settings dialog. */
  void showServerSettings();

  /** Show the manual record addition dialog. */
  void addNewRecord();

  /** Clears the database of all molecules */
  void clearDatabase();

  void runQuery();

  void setShowSelectedMolecules(bool enabled);
  void updateSelectionFilterModel();

  void showChartWidget();
  void showClusteringWidget();
  void showImportDialog();

signals:
  void connectionFailed();
};

}

#endif
