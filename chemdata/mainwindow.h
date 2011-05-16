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

#include <vtkNew.h>

class QVTKWidget;
class vtkContextView;
class vtkAnnotationLink;
class vtkEventQtSlotConnect;
class vtkExtractSelectedRows;
class vtkChartXY;
class vtkObject;
class vtkCommand;

namespace Ui {
class MainWindow;
}

namespace ChemData{

class MongoModel;

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow();
  ~MainWindow();

protected:
  MongoModel *m_model;
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

  QDialog *m_dialog;

protected slots:
  void selectionChanged();

  void chartPointClicked(vtkObject *caller, unsigned long vtk_event,
                         void* client_data, void *client_data2, vtkCommand*);

};

}

#endif
