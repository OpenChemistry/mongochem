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

#include "importcsvfiledialog.h"
#include "ui_importcsvfiledialog.h"

#include <QFile>
#include <QDebug>
#include <QFileDialog>

#include "mongodatabase.h"

ImportCsvFileDialog::ImportCsvFileDialog(QWidget *parent_)
  : QDialog(parent_),
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

    // read each data line
    int index = 0;
    while (true) {
      QString line = file.readLine().trimmed();
      if (line.isEmpty())
        break;

      ui->tableWidget->setRowCount(index + 1);

      int column = 0;
      foreach (const QString &string, line.split(",")) {
        if (column >= ui->tableWidget->columnCount())
          break;

        QTableWidgetItem *item = new QTableWidgetItem(string);
        ui->tableWidget->setItem(index, column, item);
        column++;
      }

      index++;
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
  MongoDatabase *db = MongoDatabase::instance();

  int keyColumn = 0;

  for (int i = 0; i < ui->tableWidget->rowCount(); i++) {
    QString key = ui->tableWidget->item(i, keyColumn)->text();
    MoleculeRef molecule = db->findMoleculeFromInChIKey(key.toStdString());

    for (int j = 1; j < ui->tableWidget->columnCount(); j++) {
      QString name = ui->tableWidget->horizontalHeaderItem(j)->text();
      QString value = ui->tableWidget->item(i, j)->text();

      db->setMoleculeProperty(molecule,
                              "descriptors." + name.toStdString(),
                              value.toFloat());
    }
  }

  // close the dialog
  accept();
}
