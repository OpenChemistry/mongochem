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
#include <QtCore/QMap>

#include <vtkNew.h>

#include <mongochem/gui/moleculeref.h>

class vtkAnnotationLink;
class vtkEventQtSlotConnect;

namespace mongo {
class DBClientConnection;
}

namespace Ui {
class MainWindow;
}

namespace chemkit {
class Molecule;
}

namespace MongoChem {
class AbstractImportDialog;
class AbstractVtkChartWidgetFactory;
class AbstractClusteringWidgetFactory;
class AbstractImportDialogFactory;
}

#ifdef QTTESTING
class pqTestUtility;
#endif

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
    * specified by @p ref. Replaces the current selection with the @p count
    * most similar molecules.
    *
    * By default, this uses the tanimoto coefficent of the FP2 fingerprints
    * to determine similarity between molecules.
    */
  void showSimilarMolecules(MongoChem::MoleculeRef ref, size_t count = 10);

#ifdef QTTESTING
  /**
   * Starts recording a GUI test.
   */
  void recordTest();

  /**
   * Starts playing a GUI test file. The user will be prompted to select
   * the test XML file.
   */
  void playTest();

  /**
   * Starts playing a GUI test file from @p fileName. If @p exitAfterTest
   * is @c true the application will exit when the test is finished.
   */
  void playTest(const QString &fileName, bool exitAfterTest = true);

  /**
   * Queues a test file to be played on the next iteration of the event loop.
   */
  void playTestLater(const QString &fileName, bool exitAfterTest = true);
#endif

private:
  void setupTable();
  void setupCharts();
  void showSimilarMolecules(
    const boost::shared_ptr<chemkit::Molecule> &molecule, size_t count);

  /** Connects to MongoDB */
  void connectToDatabase();

  Ui::MainWindow *m_ui;
  mongo::DBClientConnection *m_db;
  MongoModel *m_model;
  QuickQueryWidget *m_queryWidget;
  vtkNew<vtkAnnotationLink> m_annotationLink;
  vtkNew<vtkEventQtSlotConnect> m_annotationEventConnector;

  /**
   * Generic method to load factory plugins. Returns a map of the factory
   * name to a factory instance. Also adds a QAction to @p menu and connects
   * its triggered() signal to @p slot.
   */
  template<class Factory>
  QMap<QString, Factory *> loadFactoryPlugins(QMenu *menu, const char *slot);

  QMap<QString, MongoChem::AbstractVtkChartWidgetFactory *> m_charts;
  QMap<QString, MongoChem::AbstractClusteringWidgetFactory *> m_clustering;
  QMap<QString, MongoChem::AbstractImportDialogFactory *> m_importers;


  /**
   * Import dialogs are singletons and are kept around in the background
   * even after they're closed. This map stores the names and instances
   * of previously created dialogs. They are deleted when MongoChem exits.
   */
  QMap<QString, AbstractImportDialog *> m_importerInstances;

#ifdef QTTESTING
  /**
   * Used for the GUI tests when testing is enabled.
   */
  pqTestUtility *m_testUtility;
#endif

private slots:
  /** Show the molecule details dialog for the molecule referred to
   *  by @p ref. */
  void showMoleculeDetailsDialog(MongoChem::MoleculeRef ref);

  /** Show the server settings dialog. */
  void showServerSettings();

  /** Clears the database of all molecules */
  void clearDatabase();

  void runQuery();
  void resetQuery();

  void setShowSelectedMolecules(bool enabled);
  void updateSelectionFilterModel();

  void showChartWidget();
  void showClusteringWidget();
  void showImportDialog();
  void showAboutDialog();

  void fileFormatsReady();

signals:
  void connectionFailed();
};

}

#endif
