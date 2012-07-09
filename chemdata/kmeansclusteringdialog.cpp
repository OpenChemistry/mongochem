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

#include "mongodatabase.h"

#include "kmeansclusteringdialog.h"
#include "ui_kmeansclusteringdialog.h"

#include <QtGui>

#include <QVTKWidget.h>

#include <vtkNew.h>
#include <vtkTable.h>
#include <vtkProperty.h>
#include <vtkRenderer.h>
#include <vtkPointData.h>
#include <vtkDoubleArray.h>
#include <vtkPointPicker.h>
#include <vtkLookupTable.h>
#include <vtkRenderWindow.h>
#include <vtkSmartPointer.h>
#include <vtkCubeAxesActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkKMeansStatistics.h>
#include <vtkVertexGlyphFilter.h>

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
  vtkNew<vtkPoints> points;
  std::vector<MoleculeRef> molecules;
  vtkNew<vtkTable> table;
  vtkNew<vtkLookupTable> lut;
  vtkNew<vtkCubeAxesActor> cubeAxesActor;
  vtkNew<vtkPolyData> polyData;
  vtkNew<vtkVertexGlyphFilter> glyphFilter;
  vtkNew<vtkPolyDataMapper> polyDataMapper;
  vtkNew<vtkActor> polyDataActor;
  vtkNew<vtkRenderer> renderer;
  bool descriptorEnabled[3];
};

KMeansClusteringDialog::KMeansClusteringDialog(QWidget *parent_)
  : QDialog(parent_),
    d(new KMeansClusteringDialogPrivate),
    ui(new Ui::KMeansClusteringDialog)
{
  // setup ui
  ui->setupUi(this);

  // setup descriptors
  setupDescriptors();

  // setup vtk widget
  d->vtkWidget = new QVTKWidget(this);
  vtkRenderWindow *renderWindow = d->vtkWidget->GetRenderWindow();
  renderWindow->AddRenderer(d->renderer.GetPointer());

  ui->viewFrameLayout->addWidget(d->vtkWidget);

  // setup poly data
  d->polyData->SetPoints(d->points.GetPointer());

  // setup glyph filter
  d->glyphFilter->SetInputData(d->polyData.GetPointer());

  // setup poly data mapper
  d->polyDataMapper->SetInputConnection(d->glyphFilter->GetOutputPort());
  d->polyDataMapper->SetLookupTable(d->lut.GetPointer());
  d->polyDataMapper->SetScalarModeToUsePointData();
  d->polyDataMapper->SetColorMode(VTK_COLOR_MODE_MAP_SCALARS);

  // setup poly data actor
  d->polyDataActor->SetMapper(d->polyDataMapper.GetPointer());
  d->polyDataActor->GetProperty()->SetPointSize(3);
  d->polyDataActor->GetProperty()->SetInterpolationToFlat();
  d->renderer->AddActor(d->polyDataActor.GetPointer());

  // setup k-means statistics
  d->kValue = 0;
  setKValue(3);
  ui->kValueSpinBox->setValue(d->kValue);

  // setup cube axes actor
  d->cubeAxesActor->SetMapper(d->polyDataMapper.GetPointer());
  d->cubeAxesActor->SetBounds(d->polyDataMapper->GetBounds());
  d->cubeAxesActor->SetCamera(d->renderer->GetActiveCamera());
  d->renderer->AddActor(d->cubeAxesActor.GetPointer());

  // render
  d->renderer->ResetCamera();
  d->vtkWidget->update();

  // connect signals
  connect(ui->kValueSpinBox, SIGNAL(valueChanged(int)),
          this, SLOT(kValueSpinBoxChanged(int)));
  connect(ui->showCubeAxesCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(showCubeAxesToggled(bool)));
  connect(ui->showAxisLabelsCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(showAxisLabelsToggled(bool)));
  connect(ui->resetCameraButton, SIGNAL(clicked()),
          this, SLOT(resetCamera()));
  connect(ui->cubeAxesLocationComboBox, SIGNAL(currentIndexChanged(QString)),
          this, SLOT(cubeAxesLocationChanged(QString)));
  connect(ui->xDescriptorComboBox, SIGNAL(currentIndexChanged(QString)),
          this, SLOT(xDescriptorChanged(QString)));
  connect(ui->yDescriptorComboBox, SIGNAL(currentIndexChanged(QString)),
          this, SLOT(yDescriptorChanged(QString)));
  connect(ui->zDescriptorComboBox, SIGNAL(currentIndexChanged(QString)),
          this, SLOT(zDescriptorChanged(QString)));
  connect(ui->xDescriptorCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(xDescriptorEnabledToggled(bool)));
  connect(ui->yDescriptorCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(yDescriptorEnabledToggled(bool)));
  connect(ui->zDescriptorCheckBox, SIGNAL(toggled(bool)),
          this, SLOT(zDescriptorEnabledToggled(bool)));
  connect(d->vtkWidget, SIGNAL(mouseEvent(QMouseEvent*)),
          this, SLOT(viewMouseEvent(QMouseEvent*)));
}

KMeansClusteringDialog::~KMeansClusteringDialog()
{
  delete d;
  delete ui;
}

void KMeansClusteringDialog::setMolecules(const std::vector<MoleculeRef> &molecules_)
{
  MongoDatabase *db = MongoDatabase::instance();

  // set molecules
  d->molecules = molecules_;

  // update points
  d->points->SetNumberOfPoints(0);

  foreach (const MoleculeRef &ref, molecules_) {
    boost::shared_ptr<const chemkit::Molecule> molecule = db->createMolecule(ref);

    double point[3];

    for (int i = 0; i < 3; i++) {
      QString descriptor_ = this->descriptor(i);

      if (isDescriptorEnabled(i))
        point[i] = molecule->descriptor(descriptor_.toStdString()).toDouble();
      else
        point[i] = 0;
    }

    d->points->InsertNextPoint(point);
  }

  // update descriptor table
  for (int column = 0; column < 3; column++) {
    vtkNew<vtkDoubleArray> array;

    QByteArray descriptorAscii = descriptor(column).toAscii();
    array->SetName(descriptorAscii.constData());
    array->SetNumberOfComponents(1);
    array->SetNumberOfTuples(d->points->GetNumberOfPoints());

    for (vtkIdType row = 0; row < d->points->GetNumberOfPoints(); row++) {
      double point[3];
      d->points->GetPoint(row, point);
      array->SetValue(row, point[column]);
    }

    d->table->AddColumn(array.GetPointer());
  }

  // run k-means statistics
  vtkNew<vtkKMeansStatistics> kMeansStatistics;
  kMeansStatistics->RequestSelectedColumns();
  kMeansStatistics->SetAssessOption(true);
  kMeansStatistics->SetDefaultNumberOfClusters(d->kValue);
  kMeansStatistics->SetInputData(vtkStatisticsAlgorithm::INPUT_DATA,
                                 d->table.GetPointer());

  for (int i = 0; i < 3; i++) {
    QByteArray descriptorAscii = descriptor(i).toAscii();
    kMeansStatistics->SetColumnStatus(descriptorAscii.constData(), 1);
  }

  kMeansStatistics->RequestSelectedColumns();
  kMeansStatistics->Update();

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

  // update poly data
  d->polyData->SetPoints(d->points.GetPointer());
  d->polyData->GetPointData()->SetScalars(clusterAssignments.GetPointer());

  // update cube axes
  d->cubeAxesActor->SetBounds(d->polyDataMapper->GetBounds());

  // re-render
  d->renderer->ResetCamera();
  d->vtkWidget->update();
}

std::vector<MoleculeRef> KMeansClusteringDialog::molecules() const
{
  return d->molecules;
}

void KMeansClusteringDialog::setKValue(int k)
{
  d->kValue = k;

  // update lookup table
  d->lut->SetNumberOfTableValues(k);
  d->lut->Build();

  // update mapper
  d->polyDataMapper->SetScalarRange(0, k - 1);
  d->polyDataMapper->Update();
}

int KMeansClusteringDialog::kValue() const
{
  return d->kValue;
}

void KMeansClusteringDialog::setDescriptor(int index, const QString &descriptor_)
{
  QByteArray descriptorAscii = descriptor_.toAscii();

  switch (index) {
  case 0:
    d->cubeAxesActor->SetXTitle(descriptorAscii.constData());
    break;
  case 1:
    d->cubeAxesActor->SetYTitle(descriptorAscii.constData());
    break;
  case 2:
    d->cubeAxesActor->SetZTitle(descriptorAscii.constData());
    break;
  }

  // update
  setMolecules(d->molecules);
}

QString KMeansClusteringDialog::descriptor(int index)
{
  switch (index) {
  case 0:
    return ui->xDescriptorComboBox->currentText();
  case 1:
    return ui->yDescriptorComboBox->currentText();
  case 2:
    return ui->zDescriptorComboBox->currentText();
  }

  return QString();
}

void KMeansClusteringDialog::setDescriptorEnabled(int index, bool enabled)
{
  d->descriptorEnabled[index] = enabled;

  // update
  setMolecules(d->molecules);
}

bool KMeansClusteringDialog::isDescriptorEnabled(int index) const
{
  return d->descriptorEnabled[index];
}

void KMeansClusteringDialog::kValueSpinBoxChanged(int value)
{
  setKValue(value);

  // update
  setMolecules(d->molecules);
}

void KMeansClusteringDialog::showCubeAxesToggled(bool value)
{
  d->cubeAxesActor->SetVisibility(value);
  d->vtkWidget->update();
}

void KMeansClusteringDialog::showAxisLabelsToggled(bool value)
{
  d->cubeAxesActor->SetXAxisLabelVisibility(value);
  d->cubeAxesActor->SetYAxisLabelVisibility(value);
  d->cubeAxesActor->SetZAxisLabelVisibility(value);
  d->vtkWidget->update();
}

void KMeansClusteringDialog::resetCamera()
{
  d->renderer->ResetCamera();
  d->vtkWidget->update();
}

void KMeansClusteringDialog::cubeAxesLocationChanged(const QString &value)
{
  if (value == "Closest Triad")
    d->cubeAxesActor->SetFlyModeToClosestTriad();
  else if(value == "Furthest Triad")
    d->cubeAxesActor->SetFlyModeToFurthestTriad();
  else if(value == "Outer Edges")
    d->cubeAxesActor->SetFlyModeToOuterEdges();
  else if(value == "Static Triad")
    d->cubeAxesActor->SetFlyModeToStaticTriad();
  else if(value == "Static Edges")
    d->cubeAxesActor->SetFlyModeToStaticEdges();

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

void KMeansClusteringDialog::xDescriptorEnabledToggled(bool value)
{
  setDescriptorEnabled(0, value);
}

void KMeansClusteringDialog::yDescriptorEnabledToggled(bool value)
{
  setDescriptorEnabled(1, value);
}

void KMeansClusteringDialog::zDescriptorEnabledToggled(bool value)
{
  setDescriptorEnabled(2, value);
}

void KMeansClusteringDialog::viewMouseEvent(QMouseEvent *event_)
{
  if (event_->button() == Qt::LeftButton &&
      event_->type() == QMouseEvent::MouseButtonDblClick) {
    vtkNew<vtkPointPicker> picker;
    vtkRenderer *renderer = d->renderer.GetPointer();
    vtkRenderWindowInteractor *interactor = d->vtkWidget->GetInteractor();

    int *pos_ = interactor->GetEventPosition();

    if (picker->Pick(pos_[0], pos_[1], 0, renderer)) {
      vtkIdType id = picker->GetPointId();

      if (id != -1)
        emit moleculeDoubleClicked(id);
    }
  }
}

void KMeansClusteringDialog::setupDescriptors()
{
  // enable all three axes
  for (int i = 0; i < 3; i++)
    d->descriptorEnabled[i] = true;

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
  int tpsaIndex = ui->yDescriptorComboBox->findText("tpsa");
  ui->yDescriptorComboBox->setCurrentIndex(tpsaIndex);
  int vabcIndex = ui->zDescriptorComboBox->findText("vabc");
  ui->zDescriptorComboBox->setCurrentIndex(vabcIndex);

  // set cube axis labels
  d->cubeAxesActor->SetXTitle("mass");
  d->cubeAxesActor->SetYTitle("tpsa");
  d->cubeAxesActor->SetZTitle("vabc");
}
