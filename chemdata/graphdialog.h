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

#ifndef GRAPHDIALOG_H
#define GRAPHDIALOG_H

#include <QDialog>

#include <vtkNew.h>

class QVTKWidget;
class vtkChartXY;
class vtkContextView;
class vtkTable;

namespace Ui {
class GraphDialog;
}

class GraphDialog : public QDialog
{
  Q_OBJECT

public:
  explicit GraphDialog(QWidget *parent = 0);
  ~GraphDialog();

private slots:
  void showClicked();

private:
  Ui::GraphDialog *ui;
  QVTKWidget *m_vtkWidget;
  vtkNew<vtkTable> m_table;
  vtkNew<vtkContextView> m_chartView;
  vtkNew<vtkChartXY> m_chart;
};

#endif // GRAPHDIALOG_H
