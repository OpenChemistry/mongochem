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

#ifndef MONGOCHEM_ABSTRACTIMPORTDIALOG_H
#define MONGOCHEM_ABSTRACTIMPORTDIALOG_H

#include "mongochemguiexport.h"

#include <QtGui/QDialog>

namespace MongoChem {

/**
 * @class AbstractImportDialog
 *
 * The AbstractImportDialog class is an abstract base-class for
 * data import dialogs.
 */
class MONGOCHEMGUI_EXPORT AbstractImportDialog : public QDialog
{
  Q_OBJECT

public:
  /** Destroys the import dialog. */
  ~AbstractImportDialog();

protected:
  /** Creates a new import dialog. */
  explicit AbstractImportDialog(QWidget *parent_ = 0);
};

/**
 * @class AbstractImportDialogFactory
 * @brief The base class for import dialog factories in MongoChem.
 * @author Marcus D. Hanwell
 */
class MONGOCHEMGUI_EXPORT AbstractImportDialogFactory
{
public:
  virtual ~AbstractImportDialogFactory() {}

  virtual AbstractImportDialog * createInstance() = 0;
  virtual QString identifier() const = 0;
};

} // end MongoChem namespace

Q_DECLARE_INTERFACE(MongoChem::AbstractImportDialogFactory,
                    "org.openchemistry.mongochem.importdialogfactory/0.1")

#endif // MONGOCHEM_ABSTRACTIMPORTDIALOG_H
