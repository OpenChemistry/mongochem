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

#include <QMenu>
#include <QSettings>

#include <chemkit/molecule.h>

#include "mongodatabase.h"
#include "openineditorhandler.h"
#include "exportmoleculehandler.h"
#include "computationalresultsmodel.h"
#include "computationalresultstableview.h"

MoleculeDetailDialog::MoleculeDetailDialog(QWidget *parent_)
  : QDialog(parent_),
    ui(new Ui::MoleculeDetailDialog)
{
  ui->setupUi(this);

  connect(ui->closeButton, SIGNAL(clicked()), SLOT(close()));

  m_exportHandler = new ExportMoleculeHandler(this);
  connect(ui->exportButton, SIGNAL(clicked()), m_exportHandler, SLOT(exportMolecule()));

  m_openInEditorHandler = new OpenInEditorHandler(this);
  connect(ui->openInEditorButton, SIGNAL(clicked()),
          m_openInEditorHandler, SLOT(openInEditor()));

  // setup computational results tab
  m_computationalResultsModel = new ComputationalResultsModel(this);
  m_computationalResultsTableView = new ComputationalResultsTableView(this);
  m_computationalResultsTableView->setModel(m_computationalResultsModel);
  ui->computationalResultsLayout->addWidget(m_computationalResultsTableView);

  // setup annotations tab
  ui->annotationsTableWidget->setHorizontalHeaderLabels(QStringList()
                                                        << "User"
                                                        << "Comment");
  connect(ui->addAnnotationButton,
          SIGNAL(clicked()),
          this,
          SLOT(addNewAnnotation()));
  connect(ui->annotationLineEdit,
          SIGNAL(returnPressed()),
          this,
          SLOT(addNewAnnotation()));
  connect(ui->annotationsTableWidget,
          SIGNAL(itemChanged(QTableWidgetItem*)),
          this,
          SLOT(annotationItemChanged(QTableWidgetItem*)));
  ui->annotationsTableWidget->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->annotationsTableWidget,
          SIGNAL(customContextMenuRequested(const QPoint&)),
          this,
          SLOT(annotationRightClicked(const QPoint&)));
}

MoleculeDetailDialog::~MoleculeDetailDialog()
{
  delete ui;
}

void MoleculeDetailDialog::setMolecule(const MoleculeRef &moleculeRef)
{
  // store the molecule ref
  m_ref = moleculeRef;

  // load molecule from database
  MongoDatabase *db = MongoDatabase::instance();
  if (!db)
    return;

  mongo::BSONObj obj = db->fetchMolecule(moleculeRef);

  // create molecule from inchi
  boost::shared_ptr<chemkit::Molecule> molecule;
  mongo::BSONElement inchiElement = obj.getField("inchi");
  if (!inchiElement.eoo()) {
    std::string inchi = inchiElement.str();
    ui->inchiLineEdit->setText(inchi.c_str());

    // create molecule from inchi
    molecule = boost::make_shared<chemkit::Molecule>(inchi, "inchi");
  }

  // set name
  mongo::BSONElement nameElement = obj.getField("name");
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
  mongo::BSONElement formulaElement = obj.getField("formula");
  if (!formulaElement.eoo()) {
    std::string formula = formulaElement.str();
    ui->formulaLineEdit->setText(formula.c_str());
  }

  // set mass
  mongo::BSONElement massElement = obj.getField("mass");
  if (!massElement.eoo()) {
    double mass = massElement.numberDouble();
    ui->massLineEdit->setText(QString::number(mass) + "g/mol");
  }

  // set inchikey
  mongo::BSONElement inchikeyElement = obj.getField("inchikey");
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
  mongo::BSONElement diagramElement = obj.getField("diagram");
  if (!diagramElement.eoo()) {
    int length;
    const char *data_ = diagramElement.binData(length);
    QByteArray inData(data_, length);
    QImage in = QImage::fromData(inData, "PNG");
    QPixmap pix = QPixmap::fromImage(in);

    ui->diagramLabel->setText(QString());
    ui->diagramLabel->setPixmap(pix);
  }

  // set descriptors
  mongo::BSONObj descriptorsObj = obj.getObjectField("descriptors");
  if (!descriptorsObj.isEmpty()) {
    ui->descriptorsTableWidget->setColumnCount(2);
    ui->descriptorsTableWidget->setHorizontalHeaderLabels(QStringList() << "Name" << "Value");

    std::set<std::string> fields;
    descriptorsObj.getFieldNames(fields);

    ui->descriptorsTableWidget->setRowCount(static_cast<int>(fields.size()));

    int index = 0;
    foreach(const std::string &field, fields){
      mongo::BSONElement descriptorElement = descriptorsObj.getField(field);
      double value = descriptorElement.numberDouble();
      ui->descriptorsTableWidget->setItem(index, 0, new QTableWidgetItem(field.c_str()));
      ui->descriptorsTableWidget->setItem(index, 1, new QTableWidgetItem(QString::number(value)));
      index++;
    }
  }

  // setup open in editor handler
  m_openInEditorHandler->setMolecule(moleculeRef);

  // setup export handler
  m_exportHandler->setMolecule(molecule);

  // setup computational results tab
  m_computationalResultsTableView->setModel(0);
  m_computationalResultsModel->setQuery(
    QUERY("inchikey" << obj.getStringField("inchikey")));
  m_computationalResultsTableView->setModel(m_computationalResultsModel);
  m_computationalResultsTableView->resizeColumnsToContents();

  // setup annotations tab
  reloadAnnotations();
}

/// Sets the molecule to display from its InChI formula. Returns
/// \c false if the molecule could not be found in the database.
bool MoleculeDetailDialog::setMoleculeFromInchi(const std::string &inchi)
{
  MongoDatabase *db = MongoDatabase::instance();
  if (!db->isConnected()) {
    // failed to get a database connection
    return false;
  }

  MoleculeRef molecule = db->findMoleculeFromInChI(inchi);
  if (!molecule.isValid()) {
    // failed to find molecule from inchi
    return false;
  }

  setMolecule(molecule);
  return true;
}

void MoleculeDetailDialog::addNewAnnotation()
{
  QString text = ui->annotationLineEdit->text();
  if (!text.isEmpty()) {
    MongoDatabase *db = MongoDatabase::instance();
    db->addAnnotation(m_ref, text.toStdString());
    ui->annotationLineEdit->clear();
    reloadAnnotations();
  }
}

void MoleculeDetailDialog::annotationRightClicked(const QPoint &pos_)
{
  const QTableWidgetItem *item = ui->annotationsTableWidget->itemAt(pos_);
  if (item) {
    QMenu menu;
    menu.addAction("Edit", this, SLOT(editCurrentAnnotation()));
    menu.addAction("Delete", this, SLOT(deleteCurrentAnnotation()));
    menu.exec(QCursor::pos());
  }
}

void MoleculeDetailDialog::editCurrentAnnotation()
{
  int currentRow = ui->annotationsTableWidget->currentRow();
  QTableWidgetItem *item = ui->annotationsTableWidget->item(currentRow, 1);
  if (item)
    ui->annotationsTableWidget->editItem(item);
}

void MoleculeDetailDialog::deleteCurrentAnnotation()
{
  MongoDatabase *db = MongoDatabase::instance();
  if (!db)
    return;

  // delete the annotation
  int currentRow = ui->annotationsTableWidget->currentRow();
  db->deleteAnnotation(m_ref, static_cast<size_t>(currentRow));

  // reload the annotations
  reloadAnnotations();
}

void MoleculeDetailDialog::reloadAnnotations()
{
  MongoDatabase *db = MongoDatabase::instance();
  if (!db)
    return;

  mongo::BSONObj obj = db->fetchMolecule(m_ref);

  std::vector<mongo::BSONElement> annotations;
  try {
     annotations = obj["annotations"].Array();
  }
  catch (mongo::UserException) {
    // no annotations for the molecule
  }

  // don't emit itemChanged() signals when building
  ui->annotationsTableWidget->blockSignals(true);

  ui->annotationsTableWidget->setRowCount(
    static_cast<int>(annotations.size()));

  for (size_t i = 0; i < annotations.size(); i++) {
    mongo::BSONObj annotation = annotations[i].Obj();

    const char *user = annotation.getStringField("user");
    QTableWidgetItem *userItem = new QTableWidgetItem(user);
    userItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    ui->annotationsTableWidget->setItem(i, 0, userItem);

    const char *comment = annotation.getStringField("comment");
    QTableWidgetItem *commentItem = new QTableWidgetItem(comment);
    commentItem->setFlags(Qt::ItemIsEnabled
                          | Qt::ItemIsSelectable
                          | Qt::ItemIsEditable);
    ui->annotationsTableWidget->setItem(i, 1, commentItem);
  }

  // unblock signals
  ui->annotationsTableWidget->blockSignals(false);
}

void MoleculeDetailDialog::annotationItemChanged(QTableWidgetItem *item)
{
  MongoDatabase *db = MongoDatabase::instance();
  if (!db)
    return;

  int row = item->row();
  QString text = item->data(Qt::DisplayRole).toString();

  db->updateAnnotation(m_ref, static_cast<size_t>(row), text.toStdString());
}
