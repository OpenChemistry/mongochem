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
#include <boost/algorithm/string/join.hpp>

#include <QMenu>
#include <QSettings>

#include <chemkit/molecule.h>

#include "addtagdialog.h"
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

  // setup tags
  ui->tagsTextEdit->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->tagsTextEdit,
          SIGNAL(customContextMenuRequested(const QPoint&)),
          this,
          SLOT(tagsRightClicked(const QPoint&)));

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

  // set tags
  reloadTags();

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
  catch (...){
    // mongo threw a mongo::UserException or bson::assertion exception
    // which means the molecule does't have a tags array so just return
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

void MoleculeDetailDialog::reloadTags()
{
  ui->tagsTextEdit->clear();

  MongoDatabase *db = MongoDatabase::instance();
  if (!db)
    return;

  std::vector<std::string> tags = db->fetchTags(m_ref);
  std::string tagsString = boost::join(tags, ", ");
  ui->tagsTextEdit->setText(tagsString.c_str());
}

void MoleculeDetailDialog::addNewTag()
{
  std::string tag = AddTagDialog::getTag("molecules", this);
  if (tag.empty())
    return;

  MongoDatabase *db = MongoDatabase::instance();
  if (!db)
    return;

  db->addTag(m_ref, tag);
  reloadTags();
}

void MoleculeDetailDialog::removeTag(const QString &tag)
{
  MongoDatabase *db = MongoDatabase::instance();
  if (!db)
    return;

  db->removeTag(m_ref, tag.toStdString());

  reloadTags();
}

void MoleculeDetailDialog::removeSelectedTag()
{
  QAction *sender_ = qobject_cast<QAction *>(sender());
  if (!sender_)
    return;

  QString tag = sender_->data().toString();

  if (!tag.isEmpty())
    removeTag(tag);
}

void MoleculeDetailDialog::tagsRightClicked(const QPoint &pos_)
{
  QString tag;

  QTextCursor cursor_ = ui->tagsTextEdit->cursorForPosition(pos_);
  if (cursor_.atEnd()) {
    // user clicked in empty space
  }
  else {
    QTextDocument *document = ui->tagsTextEdit->document();

    // find start of tag
    QTextCursor tagStart = document->find(",",
                                          cursor_,
                                          QTextDocument::FindBackward);
    if (tagStart.isNull()) {
      // no comma, first tag in the list
      QTextCursor start(document);
      start.movePosition(QTextCursor::Start);
      tagStart = start;
    }
    else {
      // work-around for qt versions before 4.8.1 where the behavior of
      // QTextCursor::movePosition() change when the cursor has an active
      // selection (see qt commit 1b031759)
      #if QT_VERSION < 0x040801
      tagStart.movePosition(QTextCursor::PreviousCharacter);
      #endif

      // don't include the comma or space
      tagStart.movePosition(QTextCursor::NextCharacter);
      tagStart.movePosition(QTextCursor::NextCharacter);
    }

    // find end of tag
    QTextCursor tagEnd = document->find(",",
                                        cursor_);
    if (tagEnd.isNull()) {
      // no comma, last tag in the list
      QTextCursor end(document);
      end.movePosition(QTextCursor::End);
      tagEnd = end;
    }
    else {
      // don't include the comma
      tagEnd.movePosition(QTextCursor::PreviousCharacter);
    }

    // select tag
    ui->tagsTextEdit->setTextCursor(tagStart);
    while (ui->tagsTextEdit->textCursor().position() < tagEnd.position()) {
      ui->tagsTextEdit->moveCursor(QTextCursor::NextCharacter,
                                   QTextCursor::KeepAnchor);
    }

    // get tag text
    tag = ui->tagsTextEdit->textCursor().selectedText();
  }

  // create menu
  QMenu menu;
  if (!tag.isEmpty()) {
    QAction *action = menu.addAction(QString("Remove Tag '%1'").arg(tag));
    action->setData(tag);
    connect(action, SIGNAL(triggered()), SLOT(removeSelectedTag()));
    menu.addSeparator();
  }
  menu.addAction("Add New Tag", this, SLOT(addNewTag()));
  menu.exec(QCursor::pos());

  // clear selection
  ui->tagsTextEdit->setTextCursor(QTextCursor());
}
