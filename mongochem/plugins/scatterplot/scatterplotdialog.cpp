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

#include "scatterplotdialog.h"
#include "ui_scatterplotdialog.h"

#include <mongochem/gui/mongodatabase.h>

#include <QVTKInteractor.h>
#include <vtkContextView.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkContextScene.h>
#include <vtkChartXY.h>
#include <vtkAxis.h>
#include <vtkFloatArray.h>
#include <vtkStringArray.h>
#include <vtkPlot.h>
#include <QVTKWidget.h>
#include <vtkTable.h>
#include <vtkContextView.h>
#include <vtkChartXY.h>
#include <vtkAnnotationLink.h>
#include <vtkEventQtSlotConnect.h>

#include <mongochem/gui/diagramtooltipitem.h>
#include <mongochem/gui/queryprogressdialog.h>

using namespace mongo;
using MongoChem::AbstractVtkChartWidget;

ScatterPlotDialog::ScatterPlotDialog(QWidget *parent_)
  : AbstractVtkChartWidget(parent_),
    ui(new Ui::ScatterPlotDialog)
{
  ui->setupUi(this);

  connect(ui->showButton, SIGNAL(clicked()), SLOT(showClicked()));

  vtkFloatArray *xArray = vtkFloatArray::New();
  xArray->SetName("X");

  vtkFloatArray *yArray = vtkFloatArray::New();
  yArray->SetName("Y");

  m_table->AddColumn(xArray);
  m_table->AddColumn(yArray);

  xArray->Delete();
  yArray->Delete();

  m_vtkWidget = new QVTKWidget(this);

  m_chartView->SetInteractor(m_vtkWidget->GetInteractor());
  m_vtkWidget->SetRenderWindow(m_chartView->GetRenderWindow());

  m_chartView->GetScene()->AddItem(m_chart.GetPointer());
  vtkPlot *scatter = m_chart->AddPlot(vtkChart::POINTS);
  scatter->SetInputData(m_table.GetPointer(), "X", "Y");
  scatter->SetColor(0, 0, 1.0);

  // create labels array
  vtkStringArray *nameArray = vtkStringArray::New();
  nameArray->SetName("name");
  scatter->SetIndexedLabels(nameArray);
  scatter->SetTooltipLabelFormat("%i");
  nameArray->Delete();

  m_chart->SetRenderEmpty(true);
  m_chart->SetAutoAxes(false);
  m_chart->SetDrawAxesAtOrigin(true);

  // set tooltip item
  vtkNew<MongoChem::DiagramTooltipItem> tooltip;
  m_chart->SetTooltip(tooltip.GetPointer());

  QVBoxLayout *graphLayout = new QVBoxLayout;
  graphLayout->addWidget(m_vtkWidget);
  ui->graphFrame->setLayout(graphLayout);
}

ScatterPlotDialog::~ScatterPlotDialog()
{
  delete ui;
}

void ScatterPlotDialog::setSelectionLink(vtkAnnotationLink *link)
{
  // call super-class
  AbstractVtkChartWidget::setSelectionLink(link);

  // disconnect from previous annoation link
  m_annotationEventConnector->Disconnect();

  // setup annotation link
  m_chart->SetAnnotationLink(link);

  // listen to annotation changed events
  m_annotationEventConnector->Connect(link,
                                      vtkCommand::AnnotationChangedEvent,
                                      m_vtkWidget,
                                      SLOT(update()));

  // update render view
  m_vtkWidget->update();
}

void ScatterPlotDialog::showClicked()
{
  // display progress dialog
  MongoChem::QueryProgressDialog progressDialog(this);

  QString xName = ui->xComboBox->currentText().toLower();
  QString yName = ui->yComboBox->currentText().toLower();

  MongoChem::MongoDatabase *db = MongoChem::MongoDatabase::instance();

  vtkFloatArray *xArray =
    vtkFloatArray::SafeDownCast(m_table->GetColumnByName("X"));
  vtkFloatArray *yArray =
    vtkFloatArray::SafeDownCast(m_table->GetColumnByName("Y"));
  vtkStringArray *nameArray = m_chart->GetPlot(0)->GetIndexedLabels();

  // clear current data
  xArray->SetNumberOfValues(0);
  yArray->SetNumberOfValues(0);
  nameArray->SetNumberOfValues(0);

  // query for x data (100 values at a time)
  int skip = 0;
  int stride = 100;

  for (;;) {
    // update ui
    qApp->processEvents();

    // stop loading data if the user clicked cancel
    if (progressDialog.wasCanceled())
      break;

    std::auto_ptr<DBClientCursor> cursor_ =
      db->queryMolecules(mongo::Query(), stride, skip);
    if (!cursor_->more())
      break;

    while (cursor_->more()) {
      BSONObj obj = cursor_->next();
      progressDialog.setValue(0);

      // get values
      double xValue =
        obj.getFieldDotted(
          "descriptors." + xName.toStdString()).numberDouble();
      double yValue =
        obj.getFieldDotted(
          "descriptors." + yName.toStdString()).numberDouble();

      // insert into table
      xArray->InsertNextValue(static_cast<float>(xValue));
      yArray->InsertNextValue(static_cast<float>(yValue));

      // add name to tooltip array
      std::string name = obj.getField("name").str();
      nameArray->InsertNextValue(name);
    }

    // move to next block of data
    skip += stride;

    // update progress dialog
    progressDialog.setValue(xArray->GetNumberOfTuples());
  }

  // close progress dialog
  progressDialog.close();

  // refesh view
  m_table->Modified();
  m_chart->RecalculateBounds();
  m_vtkWidget->update();
}
