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

#include "quickquerywidget.h"
#include "ui_quickquerywidget.h"

#include <chemkit/molecule.h>

QuickQueryWidget::QuickQueryWidget(QWidget *parent)
  : QWidget(parent),
    ui(new Ui::QuickQueryWidget)
{
  ui->setupUi(this);

  connect(ui->queryButton, SIGNAL(clicked()), this, SIGNAL(queryClicked()));
  connect(ui->queryLineEdit, SIGNAL(returnPressed()), SIGNAL(queryClicked()));
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
  QString field = ui->fieldComboBox->currentText().toLower();
  QString value = ui->queryLineEdit->text();
  QString mode = ui->modeComboBox->currentText();

  if(value.isEmpty()){
    return mongo::Query();
  }
  else if(field == "structure"){
    chemkit::Molecule molecule(value.toStdString(), "smiles");
    int heavyAtomCount = molecule.atomCount() - molecule.atomCount("H");

    if(mode == "is"){
      return QUERY("heavyAtomCount" << heavyAtomCount).sort("heavyAtomCount");
    }
    else if(mode == "contains"){
      return QUERY("heavyAtomCount" << BSON("$gte" << heavyAtomCount)).sort("heavyAtomCount");
    }
  }
  else if(mode == "is"){
    return QUERY(field.toStdString() << value.toStdString());
  }
  else if(mode == "contains"){
    return QUERY(field.toStdString() << BSON("$regex" << value.toStdString()));
  }
}
