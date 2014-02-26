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

#include "histogramdialog.h"
#include "ui_histogramdialog.h"

#include <mongochem/gui/mongodatabase.h>

#include <QVTKInteractor.h>
#include <vtkContextView.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkContextScene.h>
#include <vtkContextView.h>
#include <vtkFloatArray.h>
#include <vtkPlot.h>
#include <QVTKWidget.h>
#include <vtkTable.h>
#include <vtkPlotBar.h>
#include <vtkChartXY.h>
#include <vtkIntArray.h>
#include <vtkAxis.h>
#include <vtkStringArray.h>
#include <vtkMathUtilities.h>
#include <vtkAnnotationLink.h>
#include <vtkEventQtSlotConnect.h>

using namespace mongo;

namespace {

// Copied from vtkScatterPlotMatrix
bool PopulateHistograms(vtkTable *input,
                        vtkTable *output,
                        vtkStringArray *s,
                        int NumberOfBins)
{
  // The output table will have the twice the number of columns, they will be
  // the x and y for input column. This is the bin centers, and the population.
  for (vtkIdType i = 0; i < s->GetNumberOfTuples(); ++i)
    {
    double minmax[2] = { 0.0, 0.0 };
    vtkStdString name(s->GetValue(i));
    vtkDataArray *in =
        vtkDataArray::SafeDownCast(input->GetColumnByName(name.c_str()));
    if (in)
      {
      // The bin values are the centers, extending +/- half an inc either side
      in->GetRange(minmax);
      if (minmax[0] == minmax[1])
        {
        minmax[1] = minmax[0] + 1.0;
        }
      double inc = (minmax[1] - minmax[0]) / (NumberOfBins) * 1.001;
      double halfInc = inc / 2.0;
      vtkSmartPointer<vtkFloatArray> extents =
          vtkFloatArray::SafeDownCast(
            output->GetColumnByName(vtkStdString(name + "_extents").c_str()));
      if (!extents)
        {
        extents = vtkSmartPointer<vtkFloatArray>::New();
        extents->SetName(vtkStdString(name + "_extents").c_str());
        }
      extents->SetNumberOfTuples(NumberOfBins);
      float *centers = static_cast<float *>(extents->GetVoidPointer(0));
      double min = minmax[0] - 0.0005 * inc + halfInc;
      for (int j = 0; j < NumberOfBins; ++j)
        {
        extents->SetValue(j, static_cast<float>(min + j * inc));
        }
      vtkSmartPointer<vtkIntArray> populations =
          vtkIntArray::SafeDownCast(
            output->GetColumnByName(vtkStdString(name + "_pops").c_str()));
      if (!populations)
        {
        populations = vtkSmartPointer<vtkIntArray>::New();
        populations->SetName(vtkStdString(name + "_pops").c_str());
        }
      populations->SetNumberOfTuples(NumberOfBins);
      int *pops = static_cast<int *>(populations->GetVoidPointer(0));
      for (int k = 0; k < NumberOfBins; ++k)
        {
        pops[k] = 0;
        }
      for (vtkIdType j = 0; j < in->GetNumberOfTuples(); ++j)
        {
        double v(0.0);
        in->GetTuple(j, &v);
        for (int k = 0; k < NumberOfBins; ++k)
          {
          if (vtkMathUtilities::FuzzyCompare(v, double(centers[k]), halfInc))
            {
            ++pops[k];
            break;
            }
          }
        }
      output->AddColumn(extents.GetPointer());
      output->AddColumn(populations.GetPointer());
      }
    }
  return true;
}

} // end anonymous namespace

HistogramDialog::HistogramDialog(QWidget *parent_)
  : MongoChem::AbstractVtkChartWidget(parent_),
    ui(new Ui::HistogramDialog)
{
  ui->setupUi(this);

  m_vtkWidget = new QVTKWidget(this);
  m_chartView->SetInteractor(m_vtkWidget->GetInteractor());
  m_vtkWidget->SetRenderWindow(m_chartView->GetRenderWindow());

  setupTable();

  vtkStringArray *names = vtkStringArray::New();
  names->InsertNextValue("tpsa");
  names->InsertNextValue("xlogp3");
  names->InsertNextValue("mass");
  names->InsertNextValue("rotatable-bonds");
  names->InsertNextValue("vabc");
  PopulateHistograms(m_table.GetPointer(), m_histogramTable.GetPointer(), names, 10);
  names->Delete();

  vtkPlotBar *plot = vtkPlotBar::New();
  m_chart->AddPlot(plot);

  // show the count in the tooltip
  plot->SetTooltipLabelFormat("Count: %y");

  plot->Delete();
  m_chartView->GetScene()->AddItem(m_chart.GetPointer());

  // set current descriptor
  setDescriptor(ui->comboBox->currentText());

  QVBoxLayout *chartLayout = new QVBoxLayout;
  chartLayout->addWidget(m_vtkWidget);
  ui->histogramFrame->setLayout(chartLayout);

  connect(ui->comboBox, SIGNAL(currentIndexChanged(QString)), SLOT(setDescriptor(QString)));
}

HistogramDialog::~HistogramDialog()
{
  delete ui;
}

void HistogramDialog::setSelectionLink(vtkAnnotationLink *link)
{
  // call super-class
  MongoChem::AbstractVtkChartWidget::setSelectionLink(link);

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

void HistogramDialog::setDescriptor(const QString &name)
{
  vtkPlotBar *plot = vtkPlotBar::SafeDownCast(m_chart->GetPlot(0));

  QByteArray extentsArrayName = (name.toLower() + "_extents").toLatin1();
  QByteArray popsArrayName = (name.toLower() + "_pops").toLatin1();

  plot->SetInputData(m_histogramTable.GetPointer(),
                     extentsArrayName.constData(),
                     popsArrayName.constData());

  m_chart->GetAxis(0)->SetTitle("Count");
  m_chart->GetAxis(1)->SetTitle(name.toLatin1().constData());

  m_chart->RecalculateBounds();
  m_chartView->ResetCamera();
  m_vtkWidget->update();
}

void HistogramDialog::setupTable()
{
  MongoChem::MongoDatabase *db = MongoChem::MongoDatabase::instance();

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

  // query molecules collection
  std::auto_ptr<DBClientCursor> cursor_ = db->queryMolecules(mongo::Query());

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
  }
}
