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

#ifndef SCATTERPLOTDIALOG_H
#define SCATTERPLOTDIALOG_H

#include <mongochem/gui/abstractvtkchartwidget.h>

#include <vtkNew.h>

class QVTKWidget;
class vtkChartXY;
class vtkContextView;
class vtkTable;
class vtkAnnotationLink;
class vtkEventQtSlotConnect;

namespace Ui {
class ScatterPlotDialog;
}

class ScatterPlotDialog : public MongoChem::AbstractVtkChartWidget
{
  Q_OBJECT

public:
  explicit ScatterPlotDialog(QWidget *parent = 0);
  ~ScatterPlotDialog();

  void setSelectionLink(vtkAnnotationLink *link);

private slots:
  void showClicked();

private:
  Ui::ScatterPlotDialog *ui;
  QVTKWidget *m_vtkWidget;
  vtkNew<vtkTable> m_table;
  vtkNew<vtkContextView> m_chartView;
  vtkNew<vtkChartXY> m_chart;
  vtkNew<vtkEventQtSlotConnect> m_annotationEventConnector;
};

#endif // SCATTERPLOTDIALOG_H
