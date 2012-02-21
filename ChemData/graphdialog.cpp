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

#include "graphdialog.h"
#include "ui_graphdialog.h"

#include <mongo/client/dbclient.h>

#include <QtCore/QSettings>

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

using namespace mongo;

GraphDialog::GraphDialog(QWidget *parent)
  : QDialog(parent),
    ui(new Ui::GraphDialog)
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
  scatter->SetInput(m_table.GetPointer(), "X", "Y");
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

  QVBoxLayout *graphLayout = new QVBoxLayout;
  graphLayout->addWidget(m_vtkWidget);
  ui->graphFrame->setLayout(graphLayout);
}

GraphDialog::~GraphDialog()
{
  delete ui;
}

void GraphDialog::showClicked()
{
  QString xName = ui->xComboBox->currentText().toLower();
  QString yName = ui->yComboBox->currentText().toLower();

  QSettings settings;
  std::string host = settings.value("hostname").toString().toStdString();
  DBClientConnection db;
  try {
    db.connect(host);
  }
  catch (DBException &e) {
    std::cerr << "Failed to connect to MongoDB: " << e.what() << std::endl;
    return;
  }

  vtkFloatArray *xArray = vtkFloatArray::SafeDownCast(m_table->GetColumnByName("X"));
  vtkFloatArray *yArray = vtkFloatArray::SafeDownCast(m_table->GetColumnByName("Y"));
  vtkStringArray *nameArray = m_chart->GetPlot(0)->GetIndexedLabels();

  // clear current data
  xArray->SetNumberOfValues(0);
  yArray->SetNumberOfValues(0);
  nameArray->SetNumberOfValues(0);

  std::string collection = settings.value("collection").toString().toStdString();

  std::string moleculeCollection = collection + ".molecules";
  std::string xCollection = collection + ".descriptors." + xName.toStdString();
  std::string yCollection = collection + ".descriptors." + yName.toStdString();

  // query for x data
  auto_ptr<DBClientCursor> xCursor = db.query(xCollection);

  while(xCursor->more()){
    BSONObj xObj = xCursor->next();
    if(xObj.isEmpty() || xObj.getField("id").eoo()){
      continue;
    }

    // get id
    OID id = xObj.getField("id").OID();

    // get x value
    double xValue = xObj.getField("value").numberDouble();

    // query for y value
    BSONObj yObj = db.findOne(yCollection, QUERY("id" << id));
    if(yObj.isEmpty()){
      std::cout << "no y value" << std::endl;
      continue;
    }

    // get y value
    double yValue = yObj.getField("value").numberDouble();

    // query for name
    BSONObj moleculeObj = db.findOne(moleculeCollection, QUERY("_id" << id));
    std::string name = moleculeObj.getField("name").str();

    // insert into table
    xArray->InsertNextValue(xValue);
    yArray->InsertNextValue(yValue);
    nameArray->InsertNextValue(name);
  }

  // refesh view
  m_table->Modified();
  m_chart->RecalculateBounds();
  m_vtkWidget->update();
}
