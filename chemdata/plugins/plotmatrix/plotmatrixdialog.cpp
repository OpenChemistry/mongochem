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
#include <vtkAnnotationLink.h>
#include <vtkEventQtSlotConnect.h>

#include <chemdata/gui/diagramtooltipitem.h>

using namespace mongo;

PlotMatrixDialog::PlotMatrixDialog(QWidget *parent_)
  : ChemData::AbstractVtkChartWidget(parent_),
    ui(new Ui::PlotMatrixDialog)
{
  ui->setupUi(this);

  m_vtkWidget = new QVTKWidget(this);
  m_chartView->SetInteractor(m_vtkWidget->GetInteractor());
  m_vtkWidget->SetRenderWindow(m_chartView->GetRenderWindow());

  setupTable();

  vtkNew<ChemData::DiagramTooltipItem> tooltip;
  m_plotMatrix->SetTooltip(tooltip.GetPointer());
  m_plotMatrix->SetIndexedLabels(
    vtkStringArray::SafeDownCast(m_table->GetColumnByName("name")));
  m_table->RemoveColumnByName("name");
  m_chartView->GetScene()->AddItem(m_plotMatrix.GetPointer());
  m_plotMatrix->SetInput(m_table.GetPointer());

  QVBoxLayout *graphLayout = new QVBoxLayout;
  graphLayout->addWidget(m_vtkWidget);
  ui->graphFrame->setLayout(graphLayout);
}

PlotMatrixDialog::~PlotMatrixDialog()
{
  delete ui;
}

void PlotMatrixDialog::setSelectionLink(vtkAnnotationLink *link)
{
  // call super-class
  ChemData::AbstractVtkChartWidget::setSelectionLink(link);

  // disconnect from previous annoation link
  m_annotationEventConnector->Disconnect();

  // setup annotation link (not sure how to do this for plot-matrix)
//  m_chart->SetAnnotationLink(link);

  // listen to annotation changed events
  m_annotationEventConnector->Connect(link,
                                      vtkCommand::AnnotationChangedEvent,
                                      m_vtkWidget,
                                      SLOT(update()));

  // update render view
  m_vtkWidget->update();
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
