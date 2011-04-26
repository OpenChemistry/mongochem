/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2011 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"

namespace ChemData {

MainWindow::MainWindow()
{
  m_ui = new Ui::MainWindow;
  m_ui->setupUi(this);

}

MainWindow::~MainWindow()
{
  delete m_ui;
  m_ui = 0;
}

}
