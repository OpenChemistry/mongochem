/******************************************************************************

  This source file is part of the ChemData project.

  Copyright 2011 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef QUICKQUERYWIDGET_H
#define QUICKQUERYWIDGET_H

#include <QWidget>

#include <mongo/client/dbclient.h>

namespace Ui {
class QuickQueryWidget;
}

class QuickQueryWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QuickQueryWidget(QWidget *parent = 0);
  ~QuickQueryWidget();

  QString field() const;
  QString value() const;
  mongo::Query query() const;

signals:
  void queryClicked();

private:
  Ui::QuickQueryWidget *ui;
};

#endif // QUICKQUERYWIDGET_H
