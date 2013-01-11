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

#include "mongodatabase.h"

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

    // first line is titles
    QString titles = file.readLine().trimmed();
    QStringList titlesList = titles.split(",", QString::SkipEmptyParts);
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
        else
          roleComboBox->setCurrentIndex(2); // ignored
      }
      else
        roleComboBox->setCurrentIndex(1); // descriptor
    }

    // read each data line
    int index = 0;
    while (true) {
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
        if(ch == '"')
          inQuotes = !inQuotes;
        else if(ch == ',' && !inQuotes){
          items.append(current);
          current.clear();
        }
        else
          current.append(ch);
      }

      // add final item
      if(!current.isEmpty())
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

      // go to next index
      ++index;
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
    qDebug() << "failed to find identifier column";
    return;
  }

  // find descriptor columns
  // (list of pairs of column indices and descriptor names)
  QList<int> descriptorColumns;
  for (int i = 0; i < ui->mappingTableWidget->rowCount(); i++) {
    QComboBox *roleComboBox =
      qobject_cast<QComboBox *>(ui->mappingTableWidget->cellWidget(i, 1));
    if (roleComboBox->currentText() == "Descriptor") {
      descriptorColumns.append(i);
    }
  }

  // import data
  for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
    QString key = ui->tableWidget->item(i, identifierColumn)->text();

    // look up molecule from identifier
    MongoChem::MoleculeRef molecule =
        db->findMoleculeFromIdentifier(key.toStdString(),
                                       identifierName.toStdString());

    if (!molecule.isValid()) {
      qDebug() << "failed to find molecule from " <<
               identifierName <<
                " identifier: " <<
                key;
      continue;
    }

    foreach (int j, descriptorColumns) {
      QString name = ui->tableWidget->horizontalHeaderItem(j)->text();
      QString value = ui->tableWidget->item(i, j)->text();

      db->setMoleculeProperty(molecule,
                              "descriptors." + name.toStdString(),
                              value.toFloat());
    }
  }

  // cleanup the dialog
  m_fileName.clear();
  ui->fileNameLineEdit->clear();
  ui->tableWidget->clear();
  ui->tableWidget->setRowCount(0);
  ui->tableWidget->setColumnCount(0);
  ui->mappingTableWidget->clear();
  ui->mappingTableWidget->setRowCount(0);
  ui->mappingTableWidget->setColumnCount(0);

  // close the dialog
  accept();
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
