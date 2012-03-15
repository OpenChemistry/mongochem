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

#ifndef PARALLELCOORDINATESDIALOG_H
#define PARALLELCOORDINATESDIALOG_H

#include <QtGui>

#include <vtkNew.h>

class QVTKWidget;
class vtkChartXY;
class vtkContextView;
class vtkTable;

namespace Ui {
  class ParallelCoordinatesDialog;
}

class ParallelCoordinatesDialog : public QDialog
{
  Q_OBJECT

public:
  explicit ParallelCoordinatesDialog(QWidget *parent = 0);
  ~ParallelCoordinatesDialog();

private:
  void setupTable();

private:
  Ui::ParallelCoordinatesDialog *ui;
  QVTKWidget *m_vtkWidget;
  vtkNew<vtkTable> m_table;
  vtkNew<vtkContextView> m_chartView;
};

#endif // PARALLELCOORDINATESDIALOG_H
