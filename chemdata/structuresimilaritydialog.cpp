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

#include "structuresimilaritydialog.h"
#include "ui_structuresimilaritydialog.h"

#include <chemkit/structuresimilaritydescriptor.h>

using namespace chemkit;

StructureSimilarityDialog::StructureSimilarityDialog(QWidget *parent)
  : QDialog(parent),
    ui(new Ui::StructureSimilarityDialog)
{
    ui->setupUi(this);

    m_graphWidget = new SimilarityGraphWidget(this);
    ui->similaritySlider->setValue(45);
    m_graphWidget->setSimilarityThreshold(ui->similaritySlider->value() / 100.f);
    ui->mainLayout->addWidget(m_graphWidget);

    connect(ui->similaritySlider, SIGNAL(sliderPressed()), SLOT(similaritySliderPressed()));
    connect(ui->similaritySlider, SIGNAL(valueChanged(int)), SLOT(similarityValueChanged(int)));
    connect(ui->similaritySlider, SIGNAL(valueChanged(int)), ui->similaritySpinBox, SLOT(setValue(int)));
    connect(ui->similaritySpinBox, SIGNAL(valueChanged(int)), ui->similaritySlider, SLOT(setValue(int)));
    connect(ui->similaritySlider, SIGNAL(sliderReleased()), SLOT(similaritySliderReleased()));
}

StructureSimilarityDialog::~StructureSimilarityDialog()
{
    delete ui;
}

void StructureSimilarityDialog::setMolecules(const std::vector<boost::shared_ptr<Molecule> > &molecules)
{
  m_molecules = molecules;

  // calculate similarity matrix
  Eigen::MatrixXf similarityMatrix;
  similarityMatrix.resize(m_molecules.size(), m_molecules.size());

  for(size_t i = 0; i < m_molecules.size(); i++){
    StructureSimilarityDescriptor descriptor;
    descriptor.setMolecule(m_molecules[i]);

    for(size_t j = i + 1; j < m_molecules.size(); j++){
      float similarity = descriptor.value(m_molecules[j].get()).toFloat();

      similarityMatrix(i, j) = similarity;
      similarityMatrix(j, i) = similarity;
    }
  }

  // recalculate graph
  m_graphWidget->setSimilarityMatrix(similarityMatrix);
}

void StructureSimilarityDialog::similaritySliderPressed()
{
  m_graphWidget->setLayoutPaused(true);
}

void StructureSimilarityDialog::similaritySliderReleased()
{
  m_graphWidget->setLayoutPaused(false);
}

void StructureSimilarityDialog::similarityValueChanged(int value)
{
  m_graphWidget->setSimilarityThreshold(value / 100.f);
}
