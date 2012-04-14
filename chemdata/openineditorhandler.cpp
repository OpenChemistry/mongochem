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

#include "openineditorhandler.h"

#include <QDir>
#include <QProcess>
#include <QTemporaryFile>

#include <chemkit/molecule.h>
#include <chemkit/moleculefile.h>
#include <chemkit/coordinatepredictor.h>
#include <chemkit/moleculegeometryoptimizer.h>

OpenInEditorHandler::OpenInEditorHandler(QObject *parent) :
    QObject(parent)
{
  // by default use the avogadro editor
  m_editorName = "avogadro";
}

OpenInEditorHandler::~OpenInEditorHandler()
{
}

void OpenInEditorHandler::setEditor(const QString &name)
{
  m_editorName = name;
}

QString OpenInEditorHandler::editor() const
{
  return m_editorName;
}

void OpenInEditorHandler::setMolecule(const boost::shared_ptr<chemkit::Molecule> &molecule)
{
  m_molecule = molecule;
}

boost::shared_ptr<chemkit::Molecule> OpenInEditorHandler::molecule() const
{
  return m_molecule;
}

void OpenInEditorHandler::openInEditor()
{
  if (!m_molecule)
    return;

  // setup temporary file
  QTemporaryFile tempFile("XXXXXX.cml");
  tempFile.setAutoRemove(false);
  if (tempFile.open()) {
    QString tempFilePath = QDir::tempPath() + QDir::separator() + tempFile.fileName();

    // predict 3d coordinates
    chemkit::CoordinatePredictor::predictCoordinates(m_molecule.get());

    // optimize 3d coordinates
    chemkit::MoleculeGeometryOptimizer optimizer(m_molecule.get());

    // try with mmff
    optimizer.setForceField("mmff");
    bool ok = optimizer.setup();

    if (!ok) {
      // try with uff
      optimizer.setForceField("uff");
      ok = optimizer.setup();
    }

    if (ok) {
      // run optimization
      for (size_t i = 0; i < 250; i++) {
        optimizer.step();

        // only check for convergance every 3 steps
        if (i % 3 == 0 && optimizer.converged() )
          break;
      }

      // write optimized coordinates to molecule
      optimizer.writeCoordinates();
    }

    // write molecule to temp file
    chemkit::MoleculeFile file(tempFilePath.toStdString());
    file.setFormat("cml");
    file.addMolecule(m_molecule);
    file.write();

    // start editor
    QProcess::startDetached(m_editorName + " " + tempFilePath);
  }
}
