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

#include <QtGui/QSplitter>
#include <QtGui/QDialog>
#include <QtGui/QVBoxLayout>
#include <QtCore/QDebug>
#include <QtCore/QProcess>

#include <QVTKWidget.h>
#include <QVTKInteractor.h>
#include <vtkContextView.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkContextScene.h>
#include <vtkChartXY.h>
#include <vtkChartParallelCoordinates.h>
#include <vtkAxis.h>
#include <vtkFloatArray.h>
#include <vtkStringArray.h>
#include <vtkPlot.h>
#include <vtkTable.h>
#include <vtkAnnotationLink.h>
#include <vtkExtractSelectedRows.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkSelection.h>

#include "mongomodel.h"

namespace ChemData {

MainWindow::MainWindow()
{
  m_ui = new Ui::MainWindow;
  m_ui->setupUi(this);

  setupTable();
  setupCharts();
  m_dialog->show();
}

MainWindow::~MainWindow()
{
  delete m_model;
  m_model = 0;
  delete m_ui;
  m_ui = 0;
}

void MainWindow::setupTable()
{
  m_model = new MongoModel(this);
  m_ui->tableView->setModel(m_model);

  m_ui->tableView->setAlternatingRowColors(true);
  m_ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

  m_ui->tableView->setSelectionMode(QAbstractItemView::ContiguousSelection);
  //m_ui->tableView->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
  //m_ui->tableView->horizontalHeader()->setResizeMode(1, QHeaderView::Stretch);
  m_ui->tableView->horizontalHeader()->setResizeMode(2, QHeaderView::Stretch);
  m_ui->tableView->horizontalHeader()->setResizeMode(3, QHeaderView::Stretch);
  m_ui->tableView->horizontalHeader()->setStretchLastSection(true);
}

void MainWindow::setupCharts()
{
  // Create a widget for the charts to live on, with a splitter.
  m_dialog = new QDialog(this);
  m_dialog->setWindowTitle("Property Graphs");
  m_dialog->setModal(false);
  QSplitter *splitter = new QSplitter(m_dialog);
  QVBoxLayout *layout = new QVBoxLayout;
  m_dialog->setLayout(layout);
  layout->addWidget(splitter);
  m_dialog->setGeometry(0, 0, 800, 600);

  QSplitter *splitter2 = new QSplitter(m_dialog);
  QSplitter *splitter3 = new QSplitter(m_dialog);

  m_vtkWidget = new QVTKWidget();
  m_chartView->SetInteractor(m_vtkWidget->GetInteractor());
  m_vtkWidget->SetRenderWindow(m_chartView->GetRenderWindow());
  m_vtkWidget->setGeometry(0, 0, 800, 600);

  vtkNew<vtkChartXY> chart;
  m_chartView->GetScene()->AddItem(chart.GetPointer());
  chart->SetRenderEmpty(true);
  chart->SetAutoAxes(false);
  chart->SetDrawAxesAtOrigin(true);

  vtkNew<vtkTable> table;
  m_model->results(table.GetPointer());
  vtkPlot *scatter = chart->AddPlot(vtkChart::POINTS);
  scatter->SetInput(table.GetPointer(), 2, 3);
  scatter->SetLabel("MLR");
  scatter->SetIndexedLabels(
        vtkStringArray::SafeDownCast(table->GetColumnByName("CAS")));
  scatter = chart->AddPlot(vtkChart::POINTS);
  scatter->SetInput(table.GetPointer(), 2, 4);
  scatter->SetLabel("RF");
  scatter->SetIndexedLabels(
        vtkStringArray::SafeDownCast(table->GetColumnByName("CAS")));
  scatter->SetColor(0, 0, 1.0);

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
  parallel->Update();
  parallel->GetAxis(2)->SetTitle("MLR");
  parallel->GetAxis(3)->SetTitle("RF");


  m_vtkWidget3 = new QVTKWidget();
  m_chartView3->SetInteractor(m_vtkWidget3->GetInteractor());
  m_vtkWidget3->SetRenderWindow(m_chartView3->GetRenderWindow());
  vtkNew<vtkChartXY> chart2;
  m_chart3 = chart2.GetPointer();
  m_chartView3->GetScene()->AddItem(chart2.GetPointer());
  chart2->SetRenderEmpty(true);
  chart2->SetAutoAxes(false);
  chart2->SetDrawAxesAtOrigin(true);

  chart->SetAnnotationLink(m_link.GetPointer());
  parallel->SetAnnotationLink(m_link.GetPointer());

  // How about a chart showing only the selected subset of the data?!?
  m_extract->SetInputConnection(0, table->GetProducerPort());
  m_extract->SetInputConnection(1,
                                vtkSelection::SafeDownCast(
                                  m_link->GetOutputDataObject(2))
                                ->GetProducerPort());

  m_extract->Update();
  vtkTable *selection = m_extract->GetOutput();
  scatter = chart2->AddPlot(vtkChart::POINTS);
  scatter->SetInput(selection, 2, 3);
  scatter->SetLabel("MLR");
  scatter->SetIndexedLabels(
        vtkStringArray::SafeDownCast(table->GetColumnByName("CAS")));
  scatter = chart2->AddPlot(vtkChart::POINTS);
  scatter->SetInput(selection, 2, 4);
  scatter->SetLabel("RF");
  scatter->SetIndexedLabels(
        vtkStringArray::SafeDownCast(table->GetColumnByName("CAS")));
  scatter->SetColor(0, 0, 1.0);
  chart2->SetShowLegend(true);
  chart2->GetAxis(vtkAxis::LEFT)->SetTitle("Predicted log(Sw)");
  chart2->GetAxis(vtkAxis::BOTTOM)->SetTitle("Observed log(Sw)");

  Connector->Connect(chart.GetPointer(),
                     vtkCommand::SelectionChangedEvent,
                     this, SLOT(selectionChanged())
                     );
  Connector2->Connect(parallel.GetPointer(),
                      vtkCommand::SelectionChangedEvent,
                      this, SLOT(selectionChanged())
                      );

  Connector->Connect(chart.GetPointer(),
                     vtkCommand::InteractionEvent,
                     this, SLOT(chartPointClicked(vtkObject*,ulong,void*,
                                                  void*, vtkCommand*))
                     );
  Connector->Connect(chart2.GetPointer(),
                     vtkCommand::InteractionEvent,
                     this, SLOT(chartPointClicked(vtkObject*,ulong,void*,
                                                  void*, vtkCommand*))
                     );

  splitter2->addWidget(m_vtkWidget);
  splitter2->addWidget(m_vtkWidget2);

  splitter3->addWidget(m_vtkWidget3);

  splitter->addWidget(splitter2);
  splitter->addWidget(splitter3);

  splitter->setOrientation(Qt::Vertical);
}

void MainWindow::selectionChanged()
{
  m_extract->Update();

  m_extract->Update();
  m_chart3->RecalculateBounds();
  m_vtkWidget3->update();

  m_vtkWidget->update();
  m_vtkWidget2->update();
}

void MainWindow::chartPointClicked(vtkObject *, unsigned long,
                                   void*, void *client_data2,
                                   vtkCommand*)
{
  vtkChartPlotData *plot = static_cast<vtkChartPlotData*>(client_data2);
  qDebug() << "Series Name:" << plot->SeriesName.c_str()
           << "Index:" << plot->Index;

  m_ui->tableView->selectRow(plot->Index);

/*  emit pointClicked(QString(plot->SeriesName.c_str()),
                    Vector2f(plot->Position.GetData()),
                    Vector2i(plot->ScreenPosition.GetData()),
                    plot->Index); */
}

}
