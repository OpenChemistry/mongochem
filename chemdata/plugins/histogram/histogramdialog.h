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

#ifndef HISTOGRAMDIALOG_H
#define HISTOGRAMDIALOG_H

#include <chemdata/gui/abstractvtkchartwidget.h>

#include <vtkNew.h>

class QVTKWidget;
class vtkChartXY;
class vtkContextView;
class vtkTable;
class vtkAnnotationLink;
class vtkEventQtSlotConnect;

namespace Ui {
class HistogramDialog;
}

class HistogramDialog : public ChemData::AbstractVtkChartWidget
{
  Q_OBJECT

public:
  explicit HistogramDialog(QWidget *parent = 0);
  ~HistogramDialog() CHEMDATA_OVERRIDE;

  void setSelectionLink(vtkAnnotationLink *link) CHEMDATA_OVERRIDE;

private slots:
  void setDescriptor(const QString &name);

private:
  void setupTable();

private:
  Ui::HistogramDialog *ui;
  QVTKWidget *m_vtkWidget;
  vtkNew<vtkTable> m_table;
  vtkNew<vtkTable> m_histogramTable;
  vtkNew<vtkContextView> m_chartView;
  vtkNew<vtkChartXY> m_chart;
  vtkNew<vtkEventQtSlotConnect> m_annotationEventConnector;
};

#endif // HISTOGRAMDIALOG_H
