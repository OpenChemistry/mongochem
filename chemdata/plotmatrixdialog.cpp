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

#include "plotmatrixdialog.h"
#include "ui_plotmatrixdialog.h"

#include <mongo/client/dbclient.h>

#include <QtCore/QSettings>

#include <QVTKInteractor.h>
#include <vtkContextView.h>
#include <QVTKWidget.h>
#include <vtkTable.h>
#include <vtkScatterPlotMatrix.h>
#include <vtkContextScene.h>
#include <vtkFloatArray.h>
#include <vtkStringArray.h>

#include "diagramtooltipitem.h"

using namespace mongo;

PlotMatrixDialog::PlotMatrixDialog(QWidget *parent_)
  : QDialog(parent_),
    ui(new Ui::PlotMatrixDialog)
{
  ui->setupUi(this);

  m_vtkWidget = new QVTKWidget(this);
  m_chartView->SetInteractor(m_vtkWidget->GetInteractor());
  m_vtkWidget->SetRenderWindow(m_chartView->GetRenderWindow());

  setupTable();

  vtkScatterPlotMatrix *plotMatrix = vtkScatterPlotMatrix::New();
  vtkNew<DiagramTooltipItem> tooltip;
  plotMatrix->SetTooltip(tooltip.GetPointer());
  plotMatrix->SetIndexedLabels(
    vtkStringArray::SafeDownCast(m_table->GetColumnByName("name")));
  m_table->RemoveColumnByName("name");
  m_chartView->GetScene()->AddItem(plotMatrix);
  plotMatrix->SetInput(m_table.GetPointer());
  plotMatrix->Delete();

  QVBoxLayout *graphLayout = new QVBoxLayout;
  graphLayout->addWidget(m_vtkWidget);
  ui->graphFrame->setLayout(graphLayout);
}

PlotMatrixDialog::~PlotMatrixDialog()
{
  delete ui;
}

void PlotMatrixDialog::setupTable()
{
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

  // hard coded (for now) descriptor names
  const char *descriptors[] = {"tpsa",
                               "xlogp3",
                               "mass",
                               "rotatable-bonds",
                               "vabc"};
  size_t descriptorCount = sizeof(descriptors) / sizeof(*descriptors);

  // add table columns
  for(size_t i = 0; i < descriptorCount; i++){
    vtkFloatArray *array = vtkFloatArray::New();
    array->SetName(descriptors[i]);
    m_table->AddColumn(array);
    array->Delete();
  }

  // create labels array
  vtkNew<vtkStringArray> nameArray;
  nameArray->SetName("name");
  m_table->AddColumn(nameArray.GetPointer());

  // query molecules collection
  std::string collection = settings.value("collection").toString().toStdString();
  std::string moleculesCollection = collection + ".molecules";
  auto_ptr<DBClientCursor> cursor_ = db.query(moleculesCollection);

  while(cursor_->more()){
    BSONObj obj = cursor_->next();
    if(obj.isEmpty()){
      continue;
    }

    for(size_t i = 0; i < descriptorCount; i++){
      BSONElement value = obj.getFieldDotted(std::string("descriptors.") + descriptors[i]);
      vtkFloatArray *array = vtkFloatArray::SafeDownCast(m_table->GetColumn(i));
      array->InsertNextValue(static_cast<float>(value.numberDouble()));
    }

    BSONElement nameElement = obj.getField("name");
    std::string name;
    if(!nameElement.eoo())
      name = nameElement.str();
    nameArray->InsertNextValue(name);
  }
}
