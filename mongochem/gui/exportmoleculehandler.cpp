/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2011-2013 Kitware, Inc.

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

#include "cjsonexporter.h"
#include "mongodatabase.h"
#include "moleculeref.h"

#include <avogadro/core/molecule.h>
#include <avogadro/io/fileformatmanager.h>

namespace MongoChem {

using Avogadro::Core::Molecule;
using Avogadro::Io::FileFormatManager;

ExportMoleculeHandler::ExportMoleculeHandler(QObject *parent_)
  : QObject(parent_)
{
}

ExportMoleculeHandler::~ExportMoleculeHandler()
{
}

void ExportMoleculeHandler::setMolecule(const MoleculeRef &molecule)
{
  m_molecule = molecule;
}

void ExportMoleculeHandler::exportMolecule()
{
  if (m_molecule.isValid()) {
    // Retrieve the molecule from the database.
    MongoDatabase *db = MongoDatabase::instance();
    mongo::BSONObj moleculeObj = db->fetchMolecule(m_molecule);

    Avogadro::Core::Molecule mol;

    // Check for atoms in the molecule.
    if (moleculeObj.hasField("atoms")) {
      // Generate chemical json string.
      std::string cjson = CjsonExporter::toCjson(moleculeObj);

      // Create a molecule object for export.
      FileFormatManager::instance().readString(mol, cjson, "cjson");
    }
    else if (moleculeObj.hasField("inchi")) {
      // Attempt to load from InChI otherwise.
      FileFormatManager::instance().readString(mol,
                                               moleculeObj.getStringField("inchi"),
                                               "inchi");
    }

    // get molecule name to set default file name
    QString moleculeName = moleculeObj.getStringField("name");

    if (moleculeName.isEmpty())
      moleculeName = "molecule";

    // Pop-up dialog and get file name
    QString fileName = QFileDialog::getSaveFileName(0,
                                                    tr("Save File"),
                                                    QDir::currentPath() +
                                                    QDir::separator() +
                                                    moleculeName + ".cml");

    if (fileName.isEmpty())
      return;

    bool ok = FileFormatManager::instance().writeFile(mol,
                                                      fileName.toStdString());
    if(!ok) {
      // writing failed, display an error dialog
      QString errorMessage =
          QString("Failed to write molecule to file (%1).").arg(fileName);

      QMessageBox::critical(0, "Molecule Export Failed", errorMessage);
    }
  }
}

} // end MongoChem namespace
