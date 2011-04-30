/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2011 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QVTKWidget.h>
#include <QVTKInteractor.h>
#include <vtkContextView.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkContextScene.h>
#include <vtkChartXY.h>
#include <vtkChartParallelCoordinates.h>
#include <vtkAxis.h>
#include <vtkFloatArray.h>
#include <vtkPlot.h>
#include <vtkTable.h>
#include <vtkAnnotationLink.h>

#include "mongomodel.h"

namespace ChemData {

MainWindow::MainWindow()
{
  m_ui = new Ui::MainWindow;
  m_ui->setupUi(this);
  m_model = new MongoModel(this);
  m_ui->tableView->setModel(m_model);

  m_vtkWidget = new QVTKWidget();
  m_chartView->SetInteractor(m_vtkWidget->GetInteractor());
  m_vtkWidget->SetRenderWindow(m_chartView->GetRenderWindow());
  m_vtkWidget->setGeometry(0, 0, 800, 600);


  vtkNew<vtkChartXY> chart;
  m_chartView->GetScene()->AddItem(chart.GetPointer());
  chart->SetRenderEmpty(true);
  chart->SetAutoAxes(false);

  vtkNew<vtkTable> table;
  m_model->results(table.GetPointer());
  vtkPlot *scatter = chart->AddPlot(vtkChart::POINTS);
  scatter->SetInput(table.GetPointer(), 2, 3);
  scatter->SetLabel("MLR");
  scatter = chart->AddPlot(vtkChart::POINTS);
  scatter->SetInput(table.GetPointer(), 2, 4);
  scatter->SetLabel("RF");

  chart->SetShowLegend(true);
  chart->GetAxis(vtkAxis::LEFT)->SetTitle("Predicted log(Sw)");
  chart->GetAxis(vtkAxis::BOTTOM)->SetTitle("Observed log(Sw)");

  m_vtkWidget2 = new QVTKWidget();
  m_chartView2->SetInteractor(m_vtkWidget2->GetInteractor());
  m_vtkWidget2->SetRenderWindow(m_chartView2->GetRenderWindow());
  m_vtkWidget2->setGeometry(100, 100, 800, 600);

  vtkNew<vtkChartParallelCoordinates> parallel;
  m_chartView2->GetScene()->AddItem(parallel.GetPointer());
  parallel->GetPlot(0)->SetInput(table.GetPointer());
  parallel->SetColumnVisibilityAll(false);
  parallel->SetColumnVisibility("Set", true);
  parallel->SetColumnVisibility("Observed", true);
  parallel->SetColumnVisibility("Predicted log Sw (MLR)", true);
  parallel->SetColumnVisibility("Predicted log Sw (RF)", true);

  chart->SetAnnotationLink(m_link.GetPointer());
  parallel->SetAnnotationLink(m_link.GetPointer());

  m_vtkWidget->show();
  m_vtkWidget2->show();
}

MainWindow::~MainWindow()
{
  delete m_model;
  m_model = 0;
  delete m_ui;
  m_ui = 0;
}

}
