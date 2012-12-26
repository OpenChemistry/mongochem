/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "mongodatabase.h"

#include "kmeansclusteringdialog.h"
#include "ui_kmeansclusteringdialog.h"

#include <QtGui/QProgressDialog>
#include <QtGui/QStandardItemModel>

#include <QVTKWidget.h>

#include <vtkNew.h>
#include <vtkTable.h>
#include <vtkChartXYZ.h>
#include <vtkIntArray.h>
#include <vtkFloatArray.h>
#include <vtkContextView.h>
#include <vtkLookupTable.h>
#include <vtkContextScene.h>
#include <vtkPlotPoints3D.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkKMeansStatistics.h>

#include <boost/array.hpp>

#include <chemkit/molecule.h>
#include <chemkit/moleculardescriptor.h>

// avoid shadow warnings from Qt's foreach by using Boost's
#undef foreach
#include <boost/foreach.hpp>
#define foreach BOOST_FOREACH

class KMeansClusteringDialogPrivate
{
public:
  QVTKWidget *vtkWidget;
  int kValue;
  std::vector<MongoChem::MoleculeRef> molecules;
  vtkSmartPointer<vtkTable> table;
  vtkNew<vtkContextView> chartView;
  vtkNew<vtkChartXYZ> chart;
  vtkPlotPoints3D *plot;
  boost::array<std::string, 3> descriptors;
  vtkNew<vtkLookupTable> lut;
};

KMeansClusteringDialog::KMeansClusteringDialog(QWidget *parent_)
  : MongoChem::AbstractClusteringWidget(parent_),
    d(new KMeansClusteringDialogPrivate),
    ui(new Ui::KMeansClusteringDialog)
{
  // setup ui
  ui->setupUi(this);

  // setup descriptors
  setupDescriptors();

  // setup vtk widget
  d->vtkWidget = new QVTKWidget(this);
  d->chartView->SetInteractor(d->vtkWidget->GetInteractor());
  d->vtkWidget->SetRenderWindow(d->chartView->GetRenderWindow());
  ui->viewFrameLayout->addWidget(d->vtkWidget);

  // setup chart xyz
  d->plot = 0;
  d->chart->SetFitToScene(true);
  d->chart->SetDecorateAxes(true);
  d->chart->SetGeometry(vtkRectf(0, 0, 600, 600));

  // setup chart view
  d->chartView->GetScene()->AddItem(d->chart.GetPointer());

  // setup k-means statistics
  d->kValue = 3;
  ui->kValueSpinBox->setValue(d->kValue);

  // render
  d->vtkWidget->update();

  // connect signals
  connect(ui->kValueSpinBox, SIGNAL(valueChanged(int)),
          this, SLOT(kValueSpinBoxChanged(int)));
  connect(ui->xDescriptorComboBox, SIGNAL(currentIndexChanged(QString)),
          this, SLOT(xDescriptorChanged(QString)));
  connect(ui->yDescriptorComboBox, SIGNAL(currentIndexChanged(QString)),
          this, SLOT(yDescriptorChanged(QString)));
  connect(ui->zDescriptorComboBox, SIGNAL(currentIndexChanged(QString)),
          this, SLOT(zDescriptorChanged(QString)));
  connect(d->vtkWidget, SIGNAL(mouseEvent(QMouseEvent*)),
          this, SLOT(viewMouseEvent(QMouseEvent*)));
}

KMeansClusteringDialog::~KMeansClusteringDialog()
{
  delete d;
  delete ui;
}

void KMeansClusteringDialog::setMolecules(const std::vector<MongoChem::MoleculeRef> &molecules_)
{
  // call super-class
  MongoChem::AbstractClusteringWidget::setMolecules(molecules_);

  // display progress dialog
  QProgressDialog progressDialog("Calculating Descriptors",
                                 "Cancel",
                                 0,
                                 molecules_.size(),
                                 this);

  // pop-up progress dialog immediately
  progressDialog.setMinimumDuration(0);

  // connect to the database
  MongoChem::MongoDatabase *db = MongoChem::MongoDatabase::instance();

  // set molecules
  d->molecules = molecules_;

  // remove old plot
  if (d->plot) {
    d->chart->ClearPlots();
    d->plot->Delete();
    d->plot = 0;
  }

  // create table for the descriptors
  d->table = vtkSmartPointer<vtkTable>::New();

  // setup descriptor arrays
  boost::array<vtkFloatArray *, 3> arrays;
  for (size_t i = 0; i < 3; i++) {
    vtkFloatArray *array = vtkFloatArray::New();
    array->SetName(d->descriptors[i].c_str());
    arrays[i] = array;
    d->table->AddColumn(array);
    array->Delete();
  }

  // setup color array
  vtkIntArray *colorArray = vtkIntArray::New();
  colorArray->SetName("color");
  colorArray->SetNumberOfComponents(1);
  d->table->AddColumn(colorArray);
  colorArray->Delete();

  // calculate descriptors
  foreach (const MongoChem::MoleculeRef &ref, molecules_) {
    // stop calculating if the user clicked cancel
    if(progressDialog.wasCanceled())
      break;

    // create molecule object
    boost::shared_ptr<const chemkit::Molecule> molecule = db->createMolecule(ref);

    // calculate descriptors for the molecule
    for (size_t i = 0; i < 3; i++)
      arrays[i]->InsertNextValue(molecule->descriptor(d->descriptors[i]).toFloat());

    // set cluster to 0 (will be set to its real value later by running k-means)
    colorArray->InsertNextValue(0);

    // update progress dialog
    progressDialog.setValue(progressDialog.value() + 1);
  }

  // run k-means statstics
  runKMeansStatistics();

  // setup plot
  d->plot = vtkPlotPoints3D::New();
  d->plot->SetInputData(d->table.GetPointer(),
                        d->descriptors[0],
                        d->descriptors[1],
                        d->descriptors[2],
                        "color");

  // setup chart
  d->chart->AddPlot(d->plot);

  // update bounds and display chart
  d->chart->RecalculateBounds();
  d->chart->RecalculateTransform();
  d->vtkWidget->update();
}

void KMeansClusteringDialog::setKValue(int k)
{
  // set k value
  d->kValue = k;

  // run k-means statistics
  runKMeansStatistics();

  // update plot with new colors (i wish this was easier)
  if (d->plot) {
    d->chart->ClearPlots();
    d->plot->Delete();
    d->plot = vtkPlotPoints3D::New();
    d->plot->SetInputData(d->table.GetPointer(),
                          d->descriptors[0],
                          d->descriptors[1],
                          d->descriptors[2],
                          "color");
    d->chart->AddPlot(d->plot);
    d->chart->RecalculateBounds();
    d->chart->RecalculateTransform();
    d->vtkWidget->update();
  }
}

int KMeansClusteringDialog::kValue() const
{
  return d->kValue;
}

void KMeansClusteringDialog::setDescriptor(int index, const QString &descriptor_)
{
  if (index < 0 || index > 2)
    return;

  // set descriptor name
  d->descriptors[index] = descriptor_.toStdString();

  // update
  setMolecules(d->molecules);
}

QString KMeansClusteringDialog::descriptor(int index)
{
  if (index >= 0 && index < 3)
    return QString::fromStdString(d->descriptors[index]);
  else
    return QString();
}

void KMeansClusteringDialog::kValueSpinBoxChanged(int value)
{
  setKValue(value);
}

void KMeansClusteringDialog::resetCamera()
{
  d->vtkWidget->update();
}

void KMeansClusteringDialog::xDescriptorChanged(const QString &descriptor_)
{
  setDescriptor(0, descriptor_);
}

void KMeansClusteringDialog::yDescriptorChanged(const QString &descriptor_)
{
  setDescriptor(1, descriptor_);
}

void KMeansClusteringDialog::zDescriptorChanged(const QString &descriptor_)
{
  setDescriptor(2, descriptor_);
}

void KMeansClusteringDialog::viewMouseEvent(QMouseEvent *event_)
{
  // not sure how to implement this with vtkChartXYZ
  (void) event_;

//  if (event_->button() == Qt::LeftButton &&
//      event_->type() == QMouseEvent::MouseButtonDblClick) {
//    vtkNew<vtkPointPicker> picker;
//    vtkRenderer *renderer = d->renderer.GetPointer();
//    vtkRenderWindowInteractor *interactor = d->vtkWidget->GetInteractor();

//    int *pos_ = interactor->GetEventPosition();

//    if (picker->Pick(pos_[0], pos_[1], 0, renderer)) {
//      vtkIdType id = picker->GetPointId();

//      if (id != -1)
//        emit moleculeDoubleClicked(id);
//    }
//  }
}

void KMeansClusteringDialog::setupDescriptors()
{
  // populate combo boxes
  std::vector<std::string> descriptors =
    chemkit::MolecularDescriptor::descriptors();

  // create map of descriptor dimensionality to list of descriptor names
  QMap<int, QStringList> descriptorsMap;

  foreach (const std::string &name, descriptors) {
    boost::scoped_ptr<chemkit::MolecularDescriptor>
      descriptor_(chemkit::MolecularDescriptor::create(name));

    int dimensionality = descriptor_->dimensionality();

    if (dimensionality == -1) {
      // descriptors with unknown dimensionality have a value of -1 so we
      // set it to INT_MAX in order to put them at the bottom of the list
      dimensionality = std::numeric_limits<int>::max();
    }

    descriptorsMap[dimensionality].append(name.c_str());
  }

  // list of the combo boxes for each of the x, y and z axes
  QList<QComboBox *> comboBoxes;
  comboBoxes.append(ui->xDescriptorComboBox);
  comboBoxes.append(ui->yDescriptorComboBox);
  comboBoxes.append(ui->zDescriptorComboBox);

  // insert each descriptor into each combo box grouped by dimensionality
  foreach (int key, descriptorsMap.keys()) {
    int dimensionality = key;
    const QStringList &names = descriptorsMap[key];

    foreach (QComboBox *comboBox, comboBoxes) {
      // insert title
      int titleIndex = comboBox->count();
      if (dimensionality != std::numeric_limits<int>::max())
        comboBox->addItem(tr("%1D Descriptors").arg(dimensionality));
      else
        comboBox->addItem(tr("Other Descriptors"));

      // set bold font for title and make it non-selectable
      QStandardItem *item =
        qobject_cast<QStandardItemModel *>(comboBox->model())->item(titleIndex);

      if (item) {
        QFont font_ = item->font();
        font_.setBold(true);
        item->setFont(font_);
        item->setFlags(Qt::ItemIsEnabled);
      }

      // add a separator
      comboBox->insertSeparator(comboBox->count());

      // insert name of each descriptor
      foreach (const QString &name, names)
        comboBox->addItem(name);
    }
  }

  // default to X = mass, Y = tpsa, and Z = vabc
  int massIndex = ui->xDescriptorComboBox->findText("mass");
  ui->xDescriptorComboBox->setCurrentIndex(massIndex);
  d->descriptors[0] = "mass";
  int tpsaIndex = ui->yDescriptorComboBox->findText("tpsa");
  ui->yDescriptorComboBox->setCurrentIndex(tpsaIndex);
  d->descriptors[1] = "tpsa";
  int vabcIndex = ui->zDescriptorComboBox->findText("vabc");
  ui->zDescriptorComboBox->setCurrentIndex(vabcIndex);
  d->descriptors[2] = "vabc";
}

void KMeansClusteringDialog::runKMeansStatistics()
{
  // run k-means statistics
  vtkNew<vtkKMeansStatistics> kMeansStatistics;
  kMeansStatistics->RequestSelectedColumns();
  kMeansStatistics->SetAssessOption(true);
  kMeansStatistics->SetDefaultNumberOfClusters(d->kValue);
  kMeansStatistics->SetInputData(vtkStatisticsAlgorithm::INPUT_DATA,
                                 d->table.GetPointer());

  for (size_t i = 0; i < 3; i++)
    kMeansStatistics->SetColumnStatus(d->descriptors[i].c_str(), 1);

  kMeansStatistics->RequestSelectedColumns();
  kMeansStatistics->Update();

  // assign molecules to clusters
  vtkNew<vtkIntArray> clusterAssignments;
  clusterAssignments->SetNumberOfComponents(1);

  size_t clusterCount = kMeansStatistics->GetOutput()->GetNumberOfColumns();
  std::vector<size_t> clusterSizes(clusterCount);

  for (vtkIdType r = 0; r < kMeansStatistics->GetOutput()->GetNumberOfRows(); r++) {
    vtkVariant v = kMeansStatistics->GetOutput()->GetValue(r, clusterCount - 1);

    int cluster = v.ToInt();
    clusterAssignments->InsertNextValue(cluster);
    clusterSizes[cluster]++;
  }

  // update lookup table
  d->lut->SetNumberOfTableValues(d->kValue);
  d->lut->Build();

  // update color array
  vtkIntArray *colorArray =
    vtkIntArray::SafeDownCast(d->table->GetColumnByName("color"));
  for(vtkIdType i = 0; i < colorArray->GetNumberOfTuples(); i++){
    colorArray->SetValue(i, clusterAssignments->GetValue(i));
  }
  d->table->Modified();

  // update cluster table widget
  ui->clusterTableWidget->setRowCount(d->kValue);
  for (int row = 0; row < d->kValue; row++) {
    // column 0 - cluster color
    QTableWidgetItem *colorItem = new QTableWidgetItem;

    double color[4];
    d->lut->GetTableValue(row, color);
    colorItem->setBackgroundColor(QColor::fromRgbF(color[0],
                                  color[1],
                                  color[2],
                                  0.8f));
    ui->clusterTableWidget->setItem(row, 0, colorItem);

    // column 1 - cluster number
    QTableWidgetItem *numberItem =
      new QTableWidgetItem(QString::number(row + 1));
    numberItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->clusterTableWidget->setItem(row, 1, numberItem);

    // column 2 - cluster size
    QTableWidgetItem *sizeItem =
      new QTableWidgetItem(QString::number(clusterSizes[row]));
    sizeItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->clusterTableWidget->setItem(row, 2, sizeItem);
  }
  ui->clusterTableWidget->resizeColumnsToContents();
  ui->clusterTableWidget->horizontalHeader()->setStretchLastSection(true);
}
