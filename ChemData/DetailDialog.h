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

#ifndef DetailDialog_H
#define DetailDialog_H

#include <QtGui/QDialog>

namespace ChemData {

class MongoModel;

class DetailDialog : public QDialog
{
  Q_OBJECT

public:
  DetailDialog(QWidget *parent = 0);

  void setModel(MongoModel *model);
  void setRow(int row);

protected:
  virtual void showEvent(QShowEvent * event);

  MongoModel *m_model;
  int m_row;
};

} // End of namespace

#endif // DetailDialog_H
