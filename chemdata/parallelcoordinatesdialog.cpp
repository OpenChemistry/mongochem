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

#include "parallelcoordinatesdialog.h"
#include "ui_parallelcoordinatesdialog.h"

#include <mongo/client/dbclient.h>

#include <QtCore/QSettings>

#include <QVTKInteractor.h>
#include <vtkContextView.h>
#include <QVTKWidget.h>
#include <vtkTable.h>
#include <vtkChartParallelCoordinates.h>
#include <vtkPlotParallelCoordinates.h>
#include <vtkContextScene.h>
#include <vtkFloatArray.h>

using namespace mongo;

ParallelCoordinatesDialog::ParallelCoordinatesDialog(QWidget *parent_)
  : QDialog(parent_),
    ui(new Ui::ParallelCoordinatesDialog)
{
  ui->setupUi(this);

  m_vtkWidget = new QVTKWidget(this);
  m_chartView->SetInteractor(m_vtkWidget->GetInteractor());
  m_vtkWidget->SetRenderWindow(m_chartView->GetRenderWindow());

  setupTable();

  vtkChartParallelCoordinates *chart = vtkChartParallelCoordinates::New();
  chart->GetPlot(0)->SetInputData(m_table.GetPointer());
  chart->SetColumnVisibilityAll(true);
  m_chartView->GetScene()->AddItem(chart);
  chart->Delete();

  QVBoxLayout *graphLayout = new QVBoxLayout;
  graphLayout->addWidget(m_vtkWidget);
  ui->graphFrame->setLayout(graphLayout);
}

ParallelCoordinatesDialog::~ParallelCoordinatesDialog()
{
  delete ui;
}

void ParallelCoordinatesDialog::setupTable()
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
  for (size_t i = 0; i < descriptorCount; i++) {
    vtkFloatArray *array = vtkFloatArray::New();
    array->SetName(descriptors[i]);
    m_table->AddColumn(array);
    array->Delete();
  }

  // query molecules collection
  std::string collection = settings.value("collection").toString().toStdString();
  std::string moleculesCollection = collection + ".molecules";
  auto_ptr<DBClientCursor> cursor_ = db.query(moleculesCollection);

  while (cursor_->more()) {
    BSONObj obj = cursor_->next();
    if (obj.isEmpty())
      continue;

    for (size_t i = 0; i < descriptorCount; i++) {
      BSONElement value =
        obj.getFieldDotted(std::string("descriptors.") + descriptors[i]);
      vtkFloatArray *array =
        vtkFloatArray::SafeDownCast(m_table->GetColumn(i));
      array->InsertNextValue(static_cast<float>(value.numberDouble()));
    }
  }
}
