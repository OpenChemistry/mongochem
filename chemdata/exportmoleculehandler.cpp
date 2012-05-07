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

#include "exportmoleculehandler.h"

#include <QFileDialog>
#include <QMessageBox>

#include <chemkit/moleculefile.h>

ExportMoleculeHandler::ExportMoleculeHandler(QObject *parent_)
  : QObject(parent_)
{
}

ExportMoleculeHandler::~ExportMoleculeHandler()
{
}

void ExportMoleculeHandler::setMolecule(const boost::shared_ptr<chemkit::Molecule> &molecule)
{
  m_molecule = molecule;
}

void ExportMoleculeHandler::exportMolecule()
{
  if (m_molecule) {
    // get molecule name to set default file name
    QString moleculeName = m_molecule->name().c_str();

    if (moleculeName.isEmpty()) {
      // if molecule has no name, just use 'molecule'
      moleculeName = "molecule";
    }

    // pop-up dialog and get file name
    QString fileName = QFileDialog::getSaveFileName(0,
                                                    tr("Save File"),
                                                    QDir::currentPath() +
                                                    QDir::separator() +
                                                    moleculeName + ".cml");

    if (fileName.isEmpty()) {
      // user clicked cancel, so do nothing
      return;
    }

    // convert file name to std::string
    std::string fileNameString = fileName.toStdString();

    // create file and add molecule
    chemkit::MoleculeFile file(fileNameString);
    file.addMolecule(m_molecule);

    // write file
    bool ok = file.write();
    if(!ok) {
      // writing failed, display an error dialog
      QString errorMessage =
          QString("Failed to write molecule to file (%1): %2")
          .arg(fileName)
          .arg(file.errorString().c_str());

      QMessageBox::critical(0, "Molecule Export Failed", errorMessage);
    }
  }
}
