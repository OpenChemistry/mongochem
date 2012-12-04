/******************************************************************************

  This source file is part of the ChemData project.

  Copyright 2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef PLOTMATRIXDIALOG_H
#define PLOTMATRIXDIALOG_H

#include <chemdata/gui/abstractvtkchartwidget.h>

#include <vtkNew.h>

class QVTKWidget;
class vtkChartXY;
class vtkContextView;
class vtkTable;
class vtkScatterPlotMatrix;
class vtkAnnotationLink;
class vtkEventQtSlotConnect;

namespace Ui {
    class PlotMatrixDialog;
}

class PlotMatrixDialog : public ChemData::AbstractVtkChartWidget
{
  Q_OBJECT

public:
  explicit PlotMatrixDialog(QWidget *parent = 0);
  ~PlotMatrixDialog() CHEMDATA_OVERRIDE;

  void setSelectionLink(vtkAnnotationLink *link) CHEMDATA_OVERRIDE;

private:
  void setupTable();

private:
  Ui::PlotMatrixDialog *ui;
  QVTKWidget *m_vtkWidget;
  vtkNew<vtkTable> m_table;
  vtkNew<vtkScatterPlotMatrix> m_plotMatrix;
  vtkNew<vtkContextView> m_chartView;
  vtkNew<vtkEventQtSlotConnect> m_annotationEventConnector;
};

#endif // PLOTMATRIXDIALOG_H
