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

#include "moleculedetaildialog.h"
#include "ui_moleculedetaildialog.h"

#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

#include <chemkit/molecule.h>

#include "exportmoleculehandler.h"

MoleculeDetailDialog::MoleculeDetailDialog(QWidget *parent)
  : QDialog(parent),
    ui(new Ui::MoleculeDetailDialog)
{
  ui->setupUi(this);

  connect(ui->closeButton, SIGNAL(clicked()), SLOT(close()));

  m_exportHandler = new ExportMoleculeHandler(this);
  connect(ui->exportButton, SIGNAL(clicked()), m_exportHandler, SLOT(exportMolecule()));
}

MoleculeDetailDialog::~MoleculeDetailDialog()
{
  delete ui;
}

void MoleculeDetailDialog::setMoleculeObject(mongo::BSONObj *obj)
{
  if (!obj) {
    return;
  }

  boost::shared_ptr<chemkit::Molecule> molecule;

  // set inchi
  mongo::BSONElement inchiElement = obj->getField("inchi");
  if (!inchiElement.eoo()) {
    std::string inchi = inchiElement.str();
    ui->inchiLineEdit->setText(inchi.c_str());

    // create molecule from inchi
    molecule = boost::make_shared<chemkit::Molecule>(inchi, "inchi");
  }

  // set name
  mongo::BSONElement nameElement = obj->getField("name");
  if (!nameElement.eoo()) {
    std::string name = nameElement.str();
    ui->nameLineEdit->setText(name.c_str());

    // trim name to 50 characters for group box title
    QString title(name.c_str());
    if (title.length() > 50) {
      title.truncate(47);
      title.append("...");
    }
    ui->diagramGroupBox->setTitle(title);

    // set molecule name
    if (molecule)
      molecule->setName(name);
  }

  // set formula
  mongo::BSONElement formulaElement = obj->getField("formula");
  if (!formulaElement.eoo()) {
    std::string formula = formulaElement.str();
    ui->formulaLineEdit->setText(formula.c_str());
  }

  // set mass
  mongo::BSONElement massElement = obj->getField("mass");
  if (!massElement.eoo()) {
    double mass = massElement.numberDouble();
    ui->massLineEdit->setText(QString::number(mass) + "g/mol");
  }

  // set inchikey
  mongo::BSONElement inchikeyElement = obj->getField("inchikey");
  if (!inchikeyElement.eoo()) {
    std::string inchikey = inchikeyElement.str();
    ui->inchikeyLineEdit->setText(inchikey.c_str());
  }

  // set smiles
  if (molecule) {
      std::string smiles = molecule->formula("smiles");
      ui->smilesLineEdit->setText(smiles.c_str());
  }

  // set diagram
  mongo::BSONElement diagramElement = obj->getField("diagram");
  if (!diagramElement.eoo()) {
    int length;
    const char *data = diagramElement.binData(length);
    QByteArray inData(data, length);
    QImage in = QImage::fromData(inData, "PNG");
    QPixmap pix = QPixmap::fromImage(in);

    ui->diagramLabel->setText(QString());
    ui->diagramLabel->setPixmap(pix);
  }

  // set descriptors
  mongo::BSONObj descriptorsObj = obj->getObjectField("descriptors");
  if (!descriptorsObj.isEmpty()) {
    ui->descriptorsTableWidget->setColumnCount(2);
    ui->descriptorsTableWidget->setHorizontalHeaderLabels(QStringList() << "Name" << "Value");

    std::set<std::string> fields;
    descriptorsObj.getFieldNames(fields);

    ui->descriptorsTableWidget->setRowCount(fields.size());

    size_t index = 0;
    foreach(const std::string &field, fields){
      mongo::BSONElement descriptorElement = descriptorsObj.getField(field);
      double value = descriptorElement.numberDouble();
      ui->descriptorsTableWidget->setItem(index, 0, new QTableWidgetItem(field.c_str()));
      ui->descriptorsTableWidget->setItem(index, 1, new QTableWidgetItem(QString::number(value)));
      index++;
    }
  }

  // setup export handler
  m_exportHandler->setMolecule(molecule);
}
