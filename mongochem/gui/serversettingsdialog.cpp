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

#include "serversettingsdialog.h"
#include "ui_serversettingsdialog.h"

#include <QtCore/QSettings>

namespace MongoChem{

ServerSettingsDialog::ServerSettingsDialog(QWidget *parent_)
  : QDialog(parent_),
    ui(new Ui::ServerSettingsDialog)
{
  ui->setupUi(this);

  QSettings settings;
  ui->hostLineEdit->setText(settings.value("hostname").toString());
  ui->portLineEdit->setText(settings.value("port").toString());
  ui->collectionLineEdit->setText(settings.value("collection").toString());
  ui->userNameLineEdit->setText(settings.value("user").toString());
}

ServerSettingsDialog::~ServerSettingsDialog()
{
  delete ui;
}

QString ServerSettingsDialog::host() const
{
  return ui->hostLineEdit->text();
}

QString ServerSettingsDialog::port() const
{
  return ui->portLineEdit->text();
}

QString ServerSettingsDialog::collection() const
{
  return ui->collectionLineEdit->text();
}

QString ServerSettingsDialog::userName() const
{
  return ui->userNameLineEdit->text();
}

}
