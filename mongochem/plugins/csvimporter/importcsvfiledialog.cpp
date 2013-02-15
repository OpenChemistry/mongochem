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

#include "importcsvfiledialog.h"
#include "ui_importcsvfiledialog.h"

#include <QFile>
#include <QDebug>
#include <QString>
#include <QComboBox>
#include <QFileDialog>
#include <QStringList>
#include <QMessageBox>
#include <QInputDialog>

#include <mongochem/gui/mongodatabase.h>
#include <mongochem/gui/svggenerator.h>

ImportCsvFileDialog::ImportCsvFileDialog(QWidget *parent_)
  : AbstractImportDialog(parent_),
    ui(new Ui::ImportCsvFileDialog)
{
  ui->setupUi(this);

  connect(ui->fileNameButton, SIGNAL(clicked()),
          this, SLOT(openFile()));
  connect(ui->importButton, SIGNAL(clicked()),
          this, SLOT(import()));
  connect(ui->cancelButton, SIGNAL(clicked()),
          this, SLOT(reject()));
  connect(ui->mappingTableWidget, SIGNAL(cellChanged(int, int)),
          this, SLOT(columnMappingTableCellChanged(int,int)));
  connect(ui->delimiterComboBox, SIGNAL(currentIndexChanged(int)),
          this, SLOT(delimiterComboBoxChanged(int)));
  connect(ui->customDelimiterLineEdit, SIGNAL(textChanged(const QString&)),
          this, SLOT(delimiterLineEditChanged(const QString&)));
}

ImportCsvFileDialog::~ImportCsvFileDialog()
{
  delete ui;
}

void ImportCsvFileDialog::setFileName(const QString &fileName_)
{
  if (fileName_ != m_fileName) {
    // set new file name
    m_fileName = fileName_;

    // update ui
    ui->fileNameLineEdit->setText(m_fileName);

    // load preview data
    QFile file(fileName_);
    if (!file.open(QFile::ReadOnly)) {
      qDebug() << "failed to open file: " << file.errorString();
      return;
    }

    // get separator character (default to comma if none specified)
    QChar separator = delimiterCharacter();

    // first line is titles
    QString titles = file.readLine().trimmed();
    QStringList titlesList = titles.split(separator, QString::SkipEmptyParts);
    ui->tableWidget->setColumnCount(titlesList.size());
    ui->tableWidget->setHorizontalHeaderLabels(titlesList);

    // setup column mapping table
    ui->mappingTableWidget->setRowCount(titlesList.size());
    ui->mappingTableWidget->setColumnCount(3);
    ui->mappingTableWidget->setHorizontalHeaderLabels(
      QStringList() << "Name" << "Role" << "Type"
    );

    bool foundIdentifier = false;

    for (int row = 0; row < titlesList.size(); row++) {
      QString title = titlesList[row];
      QTableWidgetItem *item = new QTableWidgetItem(title);
      ui->mappingTableWidget->setItem(row, 0, item);

      QComboBox *roleComboBox = new QComboBox;
      ui->mappingTableWidget->setCellWidget(row, 1, roleComboBox);


      QComboBox *typeComboBox = new QComboBox;
      ui->mappingTableWidget->setCellWidget(row, 2, typeComboBox);
      roleComboBox->setProperty("typeComboBox",
                                QVariant::fromValue<void *>(typeComboBox));
      roleComboBox->setProperty("csvColumnName", title);
      connect(roleComboBox, SIGNAL(currentIndexChanged(const QString&)),
              this, SLOT(updateTypeComboBox(const QString&)));

      // setup roles
      roleComboBox->addItem("Identifier");
      roleComboBox->addItem("Descriptor");
      roleComboBox->addItem("Ignored");

      // try to detect the type based on the name
      title = title.toLower();
      if (title == "inchi"
          || title == "inchikey"
          || title == "smiles"
          || title == "name") {
        if (!foundIdentifier) {
          roleComboBox->setCurrentIndex(0); // identifier
          foundIdentifier = true;
        }
        else {
          roleComboBox->setCurrentIndex(2); // ignored
        }
      }
      else {
        roleComboBox->setCurrentIndex(1); // descriptor
      }
    }

    // read first 25 data lines
    for (int index = 0; index < 25; index++) {
      // read line from file
      QString line = file.readLine().trimmed();
      if (line.isEmpty())
        break;

      // make space for the new row
      ui->tableWidget->setRowCount(index + 1);

      // parse each item in line into a list of strings
      QStringList items;
      QString current;
      bool inQuotes = false;

      foreach (const QChar &ch, line) {
        if (ch == '"') {
          inQuotes = !inQuotes;
        }
        else if (ch == separator && !inQuotes) {
          items.append(current);
          current.clear();
        }
        else {
          current.append(ch);
        }
      }

      // add final item
      if (!current.isEmpty())
        items.append(current);

      // add items to the table
      int column = 0;
      foreach (const QString &string, items) {
        if (column >= ui->tableWidget->columnCount())
          break;

        QTableWidgetItem *item = new QTableWidgetItem(string);
        ui->tableWidget->setItem(index, column, item);
        ++column;
      }
    }
  }

  ui->tableWidget->resizeColumnsToContents();
}

QString ImportCsvFileDialog::fileName() const
{
  return m_fileName;
}

void ImportCsvFileDialog::openFile()
{
  QString fileName_ = QFileDialog::getOpenFileName(this, "Open CSV File");

  if (!fileName_.isEmpty())
    setFileName(fileName_);
}

void ImportCsvFileDialog::import()
{
  MongoChem::MongoDatabase *db = MongoChem::MongoDatabase::instance();

  // find the identifier column
  int identifierColumn = -1;
  QString identifierName;
  for (int i = 0; i < ui->mappingTableWidget->rowCount(); i++) {
    QComboBox *roleComboBox =
      qobject_cast<QComboBox *>(ui->mappingTableWidget->cellWidget(i, 1));
    if (!roleComboBox)
      continue;
    if (roleComboBox->currentText() == "Identifier") {
      // set column for identifier
      identifierColumn = i;

      // get name of the identifier
      QComboBox *typeComboBox =
        qobject_cast<QComboBox *>(ui->mappingTableWidget->cellWidget(i, 2));
      identifierName = typeComboBox->currentText().toLower();

      break;
    }
  }

  if (identifierColumn == -1) {
    QMessageBox::critical(this,
                          tr("Import Error"),
                          tr("Failed to find identifer column."));
    return;
  }

  // find descriptor columns
  // (list of pairs of column indices and descriptor names)
  QList<int> descriptorColumns;
  for (int i = 0; i < ui->mappingTableWidget->rowCount(); i++) {
    QComboBox *roleComboBox =
      qobject_cast<QComboBox *>(ui->mappingTableWidget->cellWidget(i, 1));
    if (!roleComboBox)
      continue;
    if (roleComboBox->currentText() == "Descriptor") {
      descriptorColumns.append(i);
    }
  }

  // open the file
  QFile file(m_fileName);
  if (!file.open(QFile::ReadOnly)) {
    QMessageBox::critical(this,
                          tr("Error"),
                          tr("Failed to open file: %1")
                            .arg(file.errorString()));
    return;
  }

  // import data
  QChar separator = delimiterCharacter();
  int importedMoleculeCount = 0;

  // skip first line
  file.readLine();

  while (!file.atEnd()) {
    // read line from file
    QString line = file.readLine().trimmed();
    if (line.isEmpty())
      break;

    // parse each item in line into a list of strings
    QStringList items;
    QString current;
    bool inQuotes = false;

    foreach (const QChar &ch, line) {
      if (ch == '"') {
        inQuotes = !inQuotes;
      }
      else if (ch == separator && !inQuotes) {
        items.append(current);
        current.clear();
      }
      else {
        current.append(ch);
      }
    }

    // add final item
    if (!current.isEmpty())
      items.append(current);

    // look up molecule from identifier
    QString key = items.value(identifierColumn, QString());
    MongoChem::MoleculeRef molecule =
        db->findMoleculeFromIdentifier(key.toStdString(),
                                       identifierName.toStdString());

    // if not found, import the molecule
    if (!molecule) {
      molecule =
        db->importMoleculeFromIdentifier(key.toStdString(),
                                         identifierName.toStdString());

      // automatically generate diagram (if possible)
      if (molecule) {
        // create and setup svg generator
        MongoChem::SvgGenerator *svgGenerator =
          new MongoChem::SvgGenerator(this);
        svgGenerator->setInputData(key.toAscii());
        svgGenerator->setInputFormat(identifierName.toAscii());

        // listen to finished signal
        connect(svgGenerator, SIGNAL(finished(int)),
                this, SLOT(moleculeDiagramReady(int)));

        // store generator so we can clean it up later
        m_svgGenerators.insert(svgGenerator);

        // start the generation process in the background
        svgGenerator->start();
      }
    }

    // if we failed to import, print an error.
    //
    // @todo refactor this and display to the user in the gui
    if (!molecule) {
      qDebug() << "failed to import molecule from " <<
               identifierName <<
                " identifier: " <<
                key;
      continue;
    }

    // add descriptor values
    foreach (int j, descriptorColumns) {
      QString name = ui->tableWidget->horizontalHeaderItem(j)->text();
      QString value = items.value(j);

      db->setMoleculeProperty(molecule,
                              "descriptors." + name.toStdString(),
                              value.toFloat());
    }

    importedMoleculeCount++;
  }

  // clear the preview
  closeCurrentFile();

  // wait until all diagrams are generated
  while (!m_svgGenerators.isEmpty()) {
    qApp->processEvents();
  }

  // present information dialog
  QMessageBox::information(this,
                           "Import Successful",
                           tr("Imported %1 molecules and %2 descriptors")
                             .arg(importedMoleculeCount)
                             .arg(importedMoleculeCount *
                                  descriptorColumns.size()));

  // close the dialog
  accept();
}

void ImportCsvFileDialog::columnMappingTableCellChanged(int row, int column)
{
  // get table item
  QTableWidgetItem *item = ui->mappingTableWidget->item(row, column);
  if (!item)
    return;

  // update column header in preview table
  QTableWidgetItem *headerItem = ui->tableWidget->horizontalHeaderItem(row);
  if (headerItem)
    headerItem->setText(item->text());
}

void ImportCsvFileDialog::updateTypeComboBox(const QString &role)
{
  QComboBox *roleComboBox = qobject_cast<QComboBox *>(sender());
  if (!roleComboBox)
    return;

  // get the associated type combo box
  QComboBox *typeComboBox =
      static_cast<QComboBox *>(
        roleComboBox->property("typeComboBox").value<void *>());

  // get the column name
  QString column = roleComboBox->property("csvColumnName").toString();

  typeComboBox->clear();

  if (role == "Identifier") {
    typeComboBox->addItem("InChI");
    typeComboBox->addItem("InChIKey");
    typeComboBox->addItem("SMILES");
    typeComboBox->addItem("Name");
    typeComboBox->addItem("CAS");

    // try to guess the identifier type
    column = column.toLower();
    if (column == "inchi")
      typeComboBox->setCurrentIndex(0);
    else if (column == "inchikey")
      typeComboBox->setCurrentIndex(1);
    else if (column == "smiles")
      typeComboBox->setCurrentIndex(2);
    else if (column == "name")
      typeComboBox->setCurrentIndex(3);
    else if (column == "cas")
      typeComboBox->setCurrentIndex(4);
  }
  else if (role == "Descriptor") {
    typeComboBox->addItem("Numeric (Real)");
    typeComboBox->addItem("Numeric (Integral)");
    typeComboBox->addItem("Text");
  }
}

void ImportCsvFileDialog::delimiterComboBoxChanged(int index)
{
  Q_UNUSED(index)

  // reparse file (if one is open)
  if (!m_fileName.isEmpty()) {
    // store file name
    QString fileName_ = m_fileName;

    // close the current file
    closeCurrentFile();

    // re-open the file
    setFileName(fileName_);
  }
}

void ImportCsvFileDialog::delimiterLineEditChanged(const QString &text)
{
  Q_UNUSED(text)

  int otherIndex = 5; // index of 'Other' in the combo box

  if (ui->delimiterComboBox->currentIndex() != otherIndex) {
    // set the combo box to 'Other'
    ui->delimiterComboBox->setCurrentIndex(otherIndex);
  }
  else {
    // trigger update explicitly
    delimiterComboBoxChanged(otherIndex);
  }
}

QChar ImportCsvFileDialog::delimiterCharacter() const
{
  // mapping between the entries in the delimiter selection
  // combo box and the actual character to use. this should
  // be updated if the ui changes to add/remove delimiters
  switch (ui->delimiterComboBox->currentIndex()) {
  case 0:
    return ',';
  case 1:
    return ':';
  case 2:
    return '|';
  case 3:
    return '\t';
  case 4:
    return ' ';
  case 5:
    if (!ui->customDelimiterLineEdit->text().isEmpty())
      return ui->customDelimiterLineEdit->text()[0];
    else
      return ',';
  default:
    return ',';
  }
}

void ImportCsvFileDialog::closeCurrentFile()
{
  // cleanup the dialog
  m_fileName.clear();
  ui->fileNameLineEdit->clear();
  ui->tableWidget->clear();
  ui->tableWidget->setRowCount(0);
  ui->tableWidget->setColumnCount(0);
  ui->mappingTableWidget->clear();
  ui->mappingTableWidget->setRowCount(0);
  ui->mappingTableWidget->setColumnCount(0);
}

void ImportCsvFileDialog::moleculeDiagramReady(int errorCode)
{
  MongoChem::SvgGenerator *svgGenerator =
    qobject_cast<MongoChem::SvgGenerator *>(sender());
  if (!svgGenerator) {
    return;
  }

  QByteArray svg = svgGenerator->svg();

  if (errorCode != 0 || svg.isEmpty()) {
    qDebug() << "error generating svg";
    return;
  }

  // get molecule data
  QByteArray identifier = svgGenerator->inputData();
  QByteArray identifierFormat = svgGenerator->inputFormat();

  // get molecule ref
  MongoChem::MongoDatabase *db = MongoChem::MongoDatabase::instance();
  MongoChem::MoleculeRef molecule =
    db->findMoleculeFromIdentifier(identifier.constData(),
                                   identifierFormat.constData());

  // set molecule diagram
  if (molecule) {
    db->connection()->update(db->moleculesCollectionName(),
                             QUERY("_id" << molecule.id()),
                             BSON("$set" << BSON("svg" << svg.constData())),
                             true,
                             true);
  }

  // remove and delete generator
  m_svgGenerators.remove(svgGenerator);
  svgGenerator->deleteLater();
}
