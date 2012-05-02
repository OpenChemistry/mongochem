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

#ifndef KMEANSCLUSTERINGDIALOG_H
#define KMEANSCLUSTERINGDIALOG_H

#include <vector>

#include <boost/shared_ptr.hpp>

#include <vtkType.h>

#include <QDialog>
#include <QString>

#include "moleculeref.h"

namespace Ui {
class KMeansClusteringDialog;
}

namespace chemkit {
class Molecule;
}

class KMeansClusteringDialogPrivate;

class KMeansClusteringDialog : public QDialog
{
  Q_OBJECT

public:
  explicit KMeansClusteringDialog(QWidget *parent = 0);
  ~KMeansClusteringDialog();

  void setKValue(int k);
  int kValue() const;

  void setMolecules(const std::vector<MoleculeRef> &molecules);
  std::vector<MoleculeRef> molecules() const;

  void setDescriptor(int index, const QString &descriptor);
  QString descriptor(int index);
  void setDescriptorEnabled(int index, bool enabled = true);
  bool isDescriptorEnabled(int index) const;

signals:
  void moleculeDoubleClicked(vtkIdType id);

private slots:
  void kValueSpinBoxChanged(int value);
  void showCubeAxesToggled(bool value);
  void showAxisLabelsToggled(bool value);
  void resetCamera();
  void cubeAxesLocationChanged(const QString &value);
  void xDescriptorChanged(const QString &descriptor);
  void yDescriptorChanged(const QString &descriptor);
  void zDescriptorChanged(const QString &descriptor);
  void xDescriptorEnabledToggled(bool value);
  void yDescriptorEnabledToggled(bool value);
  void zDescriptorEnabledToggled(bool value);
  void viewMouseEvent(QMouseEvent *event);

private:
  void setupDescriptors();

private:
  KMeansClusteringDialogPrivate* const d;
  Ui::KMeansClusteringDialog *ui;
};

#endif // KMEANSCLUSTERINGDIALOG_H
