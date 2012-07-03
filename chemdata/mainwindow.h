/******************************************************************************

  This source file is part of the ChemData project.

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

#include <vtkNew.h>
#include <vtkType.h>

namespace mongo
{
  class DBClientConnection;
}

class QVTKWidget;
class vtkContextView;
class vtkAnnotationLink;
class vtkEventQtSlotConnect;
class vtkExtractSelectedRows;
class vtkChartXY;
class vtkObject;
class vtkCommand;

class QuickQueryWidget;

namespace Ui {
class MainWindow;
}

namespace ChemData{

class DetailDialog;
class MongoModel;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow();
  ~MainWindow();

protected:
  void setupTable();
  void setupCharts();
  void addMoleculesFromFile(const QString &fileName);

  /** Connects to MongoDB */
  void connectToDatabase();

  Ui::MainWindow *m_ui;

  vtkNew<vtkAnnotationLink> m_link;

  QVTKWidget *m_vtkWidget;
  vtkNew<vtkContextView> m_chartView;

  QVTKWidget *m_vtkWidget2;
  vtkNew<vtkContextView> m_chartView2;

  QVTKWidget *m_vtkWidget3;
  vtkNew<vtkContextView> m_chartView3;
  vtkChartXY *m_chart3;

  vtkNew<vtkEventQtSlotConnect> Connector;
  vtkNew<vtkEventQtSlotConnect> Connector2;
  vtkNew<vtkExtractSelectedRows> m_extract;

  mongo::DBClientConnection *m_db;
  DetailDialog *m_detail;
  MongoModel *m_model;
  QDialog *m_dialog;
  QuickQueryWidget *m_queryWidget;

protected slots:
  /** Show the graphs dialog. */
  void showGraphs();

  /** Show the histogram dialog. */
  void showHistogram();

  /** Show the scatter plot matrix dialog. */
  void showPlotMatrix();

  /** Show the parallel coordinates dialog. */
  void showParallelCoordinates();

  /** Show the molecule details dialog. */
  void showMoleculeDetailsDialog(const QModelIndex &index);

  /** Show the molecule details dialog given an id. */
  void showMoleculeDetailsDialog(vtkIdType id);

  /** Show the server settings dialog. */
  void showServerSettings();

  /** Show the k-means clustering dialog */
  void showKMeansClusteringDialog();

  /** Show the fingerprint similarity dialog. */
  void showFingerprintSimilarityDialog();

  /** Show the structure similarity dialog. */
  void showStructureSimilarityDialog();

  /** Show the manual record addition dialog. */
  void addNewRecord();

  /** Clears the database of all molecules */
  void clearDatabase();

  void selectionChanged();

  void chartPointClicked(vtkObject *caller, unsigned long vtk_event,
                         void* client_data, void *client_data2, vtkCommand*);

  void runQuery();

signals:
  void connectionFailed();
};

}

#endif
