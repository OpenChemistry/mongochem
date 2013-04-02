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

#include <chemkit/molecule.h>

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

  if(value_.isEmpty()){
    return mongo::Query();
  }
  else if (field_ == "tag") {
    mongo::BSONArrayBuilder builder;
    builder.append(value_.toStdString());
    return QUERY("tags" << BSON("$in" << builder.arr()));
  }
  else if(field_ == "structure"){
    chemkit::Molecule molecule(value_.toStdString(), "smiles");
    int heavyAtomCount =
      static_cast<int>(molecule.atomCount() - molecule.atomCount("H"));

    if(mode == "is"){
      return QUERY("heavyAtomCount" << heavyAtomCount).sort("heavyAtomCount");
    }
    else if(mode == "contains"){
      return QUERY("heavyAtomCount" << BSON("$gte" << heavyAtomCount)).sort("heavyAtomCount");
    }
  }
  else if(mode == "is"){
    return QUERY(field_.toStdString() << value_.toStdString());
  }
  else if(mode == "contains"){
    QString escapedValue = QRegExp::escape(value_);
    return QUERY(field_.toStdString() << BSON("$regex" <<
      escapedValue.toStdString()));
  }

  return mongo::Query();
}

} // end MongoChem namespace
