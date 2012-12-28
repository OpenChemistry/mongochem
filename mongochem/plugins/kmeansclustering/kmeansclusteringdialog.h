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

#ifndef KMEANSCLUSTERINGDIALOG_H
#define KMEANSCLUSTERINGDIALOG_H

#include <vector>

#include <vtkType.h>

#include <QString>

#include "moleculeref.h"
#include <mongochem/gui/abstractclusteringwidget.h>

namespace Ui {
class KMeansClusteringDialog;
}

namespace chemkit {
class Molecule;
}

class KMeansClusteringDialogPrivate;

class KMeansClusteringDialog : public MongoChem::AbstractClusteringWidget
{
  Q_OBJECT

public:
  explicit KMeansClusteringDialog(QWidget *parent = 0);
  ~KMeansClusteringDialog();

  void setKValue(int k);
  int kValue() const;

  void setMolecules(const std::vector<MongoChem::MoleculeRef> &molecules);

  void setDescriptor(int index, const QString &descriptor);
  QString descriptor(int index);

private slots:
  void kValueSpinBoxChanged(int value);
  void resetCamera();
  void xDescriptorChanged(const QString &descriptor);
  void yDescriptorChanged(const QString &descriptor);
  void zDescriptorChanged(const QString &descriptor);
  void viewMouseEvent(QMouseEvent *event);

private:
  void setupDescriptors();
  void runKMeansStatistics();

private:
  KMeansClusteringDialogPrivate* const d;
  Ui::KMeansClusteringDialog *ui;
};

#endif // KMEANSCLUSTERINGDIALOG_H
