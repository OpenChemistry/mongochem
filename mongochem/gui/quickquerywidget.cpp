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

#include "quickquerywidget.h"
#include "ui_quickquerywidget.h"

#include "chemkit.h"

#include <mongo/client/dbclient.h>

namespace MongoChem {

QuickQueryWidget::QuickQueryWidget(QWidget *parent_)
  : QWidget(parent_),
    ui(new Ui::QuickQueryWidget)
{
  ui->setupUi(this);

  connect(ui->queryButton, SIGNAL(clicked()), this, SIGNAL(queryClicked()));
  connect(ui->clearButton, SIGNAL(clicked()),
          this, SIGNAL(resetQueryClicked()));
  connect(ui->clearButton, SIGNAL(clicked()),
          ui->queryLineEdit, SLOT(clear()));
  connect(ui->queryLineEdit, SIGNAL(returnPressed()), SIGNAL(queryClicked()));
  connect(ui->fieldComboBox, SIGNAL(currentIndexChanged(const QString&)),
          SLOT(updateModeComboBox(const QString&)));
  connect(ui->fieldComboBox, SIGNAL(currentIndexChanged(const QString&)),
          SLOT(updatePlaceholderText(const QString&)));
}

QuickQueryWidget::~QuickQueryWidget()
{
  delete ui;
}

QString QuickQueryWidget::field() const
{
  return ui->fieldComboBox->currentText();
}

QString QuickQueryWidget::value() const
{
  return ui->queryLineEdit->text();
}

mongo::Query QuickQueryWidget::query() const
{
  QString field_ = ui->fieldComboBox->currentText().toLower();
  QString value_ = ui->queryLineEdit->text();
  QString mode = ui->modeComboBox->currentText();

  if (value_.isEmpty()){
    return mongo::Query();
  }
  else if (field_ == "tag") {
    if (mode == "is") {
      mongo::BSONArrayBuilder builder;
      builder.append(value_.toStdString());
      return QUERY("tags" << BSON("$in" << builder.arr()));
    }
    else if (mode == "contains") {
      std::stringstream function;
      function << "function()\n"
                  "{\n"
                  "  if (!this.tags)\n"
                  "    return false;\n"
                  "  for (var i = 0; i < this.tags.length; i++){\n"
                  "    if (this.tags[i].indexOf(\"" <<
                  value_.toStdString() <<
                  "\") != -1){\n"
                  "      return true;\n"
                  "    }\n"
                  "  }\n"
                  "  return false;\n"
                  "}\n";
      return QUERY("$where" << function.str());
    }
  }
  else if (field_ == "structure"){
    std::string format;
    if (value_.startsWith("InChI="))
      format = "inchi";
    else
      format = "smiles";
    int heavyAtomCount = ChemKit::heavyAtomCount(value_.toStdString(), format);

    if (mode == "is")
      return QUERY("heavyAtomCount" << heavyAtomCount).sort("heavyAtomCount");
    else if (mode == "contains")
      return QUERY("heavyAtomCount" << BSON("$gte" << heavyAtomCount)).sort("heavyAtomCount");
  }
  else if (field_ == "field") {
    return QUERY(value_.toStdString() << BSON("$exists" << (mode == "exists")));
  }
  else if (mode == "is"){
    return QUERY(field_.toStdString() << value_.toStdString());
  }
  else if (mode == "contains"){
    QString escapedValue = QRegExp::escape(value_);
    return QUERY(field_.toStdString() << BSON("$regex" <<
      escapedValue.toStdString()));
  }

  return mongo::Query();
}

void QuickQueryWidget::updateModeComboBox(const QString &field_)
{
  if (field_ == "Field") {
    ui->modeComboBox->clear();
    ui->modeComboBox->addItem("exists");
    ui->modeComboBox->addItem("not exists");
  }
  else {
    ui->modeComboBox->clear();
    ui->modeComboBox->addItem("is");
    ui->modeComboBox->addItem("contains");
  }
}

void QuickQueryWidget::updatePlaceholderText(const QString &field_)
{
    ui->queryLineEdit->setPlaceholderText(QString());

    if (field_ == tr("Structure")) {
      ui->queryLineEdit->setPlaceholderText(
        tr("SMILES or InChI (e.g. \"CO\" or \"InChI=1S/CH4O/c1-2/h2H,1H3\")"));
    }
}

} // end MongoChem namespace
