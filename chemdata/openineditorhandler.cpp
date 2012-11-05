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

#include <boost/make_shared.hpp>

#include <QDir>
#include <QProcess>
#include <QTemporaryFile>

#include <chemkit/molecule.h>
#include <chemkit/moleculefile.h>
#include <chemkit/coordinatepredictor.h>
#include <chemkit/moleculegeometryoptimizer.h>

#include "mongodatabase.h"

OpenInEditorHandler::OpenInEditorHandler(QObject *parent_) :
    QObject(parent_)
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

void OpenInEditorHandler::setMolecule(const MoleculeRef &molecule_)
{
  m_moleculeRef = molecule_;
}

MoleculeRef OpenInEditorHandler::molecule() const
{
  return m_moleculeRef;
}

void OpenInEditorHandler::openInEditor()
{
  if (!m_moleculeRef.isValid())
    return;

  MongoDatabase *db = MongoDatabase::instance();
  mongo::BSONObj moleculeObj = db->fetchMolecule(m_moleculeRef);
  mongo::BSONElement inchiElement = moleculeObj.getField("inchi");
  if (inchiElement.eoo())
    return;

  boost::shared_ptr<chemkit::Molecule> molecule_ =
    boost::make_shared<chemkit::Molecule>(inchiElement.str(), "inchi");

  // setup temporary file
  QTemporaryFile tempFile("XXXXXX.cml");
  tempFile.setAutoRemove(false);
  if (tempFile.open()) {
    QString tempFilePath = tempFile.fileName();

    // predict 3d coordinates
    chemkit::CoordinatePredictor::predictCoordinates(molecule_.get());

    // optimize 3d coordinates
    chemkit::MoleculeGeometryOptimizer optimizer(molecule_.get());

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
    file.addMolecule(molecule_);
    file.write();

    // start editor
    QProcess::startDetached(m_editorName + " " + tempFilePath);
  }
}
