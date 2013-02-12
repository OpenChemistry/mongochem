/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2013 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "queryprogressdialog.h"
#include "ui_queryprogressdialog.h"

#include <QPushButton>

namespace MongoChem {

QueryProgressDialog::QueryProgressDialog(QWidget *parent_)
  : QDialog(parent_),
    ui(new Ui::QueryProgressDialog)
{
  ui->setupUi(this);

  m_value = 0;
  m_wasCancelled = false;

  connect(ui->buttonBox->button(QDialogButtonBox::Cancel), SIGNAL(clicked()),
          this, SLOT(cancelClicked()));

  show();
}

QueryProgressDialog::~QueryProgressDialog()
{
}

void QueryProgressDialog::setValue(int value_)
{
  if (value_ != m_value) {
    m_value = value_;

    ui->label->setText(tr("Loaded %1 values").arg(value_));
  }
}

int QueryProgressDialog::value() const
{
  return m_value;
}

bool QueryProgressDialog::wasCanceled() const
{
  return m_wasCancelled;
}

void QueryProgressDialog::cancelClicked()
{
  m_wasCancelled = true;
}

} // end MongoChem namespace
