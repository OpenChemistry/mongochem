/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2013 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "batchjobmanager.h"

#include "avogadrotools.h"
#include "batchjobdecorator.h"
#include "moleculeref.h"
#include "mongodatabase.h"
#include "mongomodel.h"

#include <avogadro/molequeue/inputgenerator.h>
#include <avogadro/molequeue/inputgeneratordialog.h>
#include <avogadro/molequeue/inputgeneratorwidget.h>
#include <avogadro/qtgui/molecule.h>

#include <QtWidgets/QMessageBox>

#include <QtCore/QCoreApplication>
#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QScopedPointer>

using Avogadro::MoleQueue::BatchJob;
using Avogadro::MoleQueue::InputGenerator;
using Avogadro::MoleQueue::InputGeneratorDialog;
using Avogadro::MoleQueue::InputGeneratorWidget;
using Avogadro::QtGui::Molecule;

namespace MongoChem {

BatchJobManager *BatchJobManager::m_instance = NULL;

BatchJobManager::BatchJobManager(QObject *par) :
  QObject(par)
{
  refreshGenerators();
}

BatchJobManager::~BatchJobManager()
{
}

BatchJobManager &BatchJobManager::instance()
{
  if (!BatchJobManager::m_instance)
    BatchJobManager::m_instance = new BatchJobManager;
  return *BatchJobManager::m_instance;
}

QList<QAction *> BatchJobManager::createActions() const
{
  QList<QAction*> result;

  foreach (const QString &programName, m_scriptFiles.uniqueKeys()) {
    QStringList scripts = m_scriptFiles.values(programName);
    // Include the full path if there are multiple generators with the same name
    if (scripts.size() == 1) {
      result << createAction(programName, scripts.first());
    }
    else {
      foreach (const QString &filePath, scripts) {
        result << createAction(QString("%1 (%2)").arg(programName, filePath),
                               filePath);
      }
    }
  }

  return result;
}

BatchJobDecorator *BatchJobManager::performBatchCalculation(
    QWidget *windowParent, const QAction &action, const MongoModel &model)
{
  // The script path is encoded in the action's data object:
  if (action.data().type() != QVariant::String)
    return NULL;

  QString scriptFilePath = action.data().toString();
  BatchJobDecorator *batch = new BatchJobDecorator(scriptFilePath, this);
  // Ensure that the batch pointer will be cleaned up if we return early:
  QScopedPointer<BatchJobDecorator> cleanup(batch);
  if (!batch->inputGenerator().isValid())
    return NULL;

  // Generate warnings if this will submit more than one job, or if the query
  // results are incomplete.
  typedef std::vector<MoleculeRef> MolRefList;
  MolRefList moleculeRefs = model.molecules();
  size_t numMolecules = moleculeRefs.size();
  bool queryComplete = !model.hasMoreData();

  if (numMolecules < 1)
    return NULL;

  QString prompt;
  if (numMolecules > 1) {
    prompt = tr("Configure the calculation for the first molecule. This "
                "calculation will then be applied to all %Ln molecules.", "",
                numMolecules);
  }

  if (!queryComplete) {
    if (!prompt.isEmpty())
      prompt += "\n\n";
    prompt += tr("Warning: The calculation will only be performed on the %Ln "
                 "displayed molecule(s), not the complete query result.", "",
                 numMolecules);
  }

  // Get the first molecule that can convert cleanly for reference:
  Molecule mol;
  for (MolRefList::const_iterator it = moleculeRefs.begin(),
       itEnd = moleculeRefs.end(); it != itEnd; ++it) {
    mol = Molecule();
    if (AvogadroTools::createMolecule(*it, mol) &&
        mol.atomCount() > 0) {
      break;
    }
  }

  if (mol.atomCount() == 0) {
    QMessageBox::warning(windowParent, tr("MongoChem"),
                         tr("None of the currently displayed molecules have "
                            "3D coordinate information."),
                         QMessageBox::Ok);
    return NULL;
  }

  InputGeneratorDialog dlg(scriptFilePath, windowParent);
  dlg.setMolecule(&mol);
  dlg.widget().setBatchMode(true);
  dlg.show();

  // Show the warning over the input generator dialog.
  if (!prompt.isEmpty()) {
    QMessageBox::information(&dlg, tr("MongoChem"), prompt,
                             QMessageBox::Ok);
  }

  // Get user options
  bool canceled = !dlg.configureBatchJob(*batch);
  dlg.setMolecule(NULL);

  if (canceled)
    return NULL;

  m_batchJobs.append(batch);

  connect(batch,
          SIGNAL(jobCompleted(Avogadro::QtGui::BatchJob::BatchId,
                              Avogadro::QtGui::BatchJob::JobState)),
          SLOT(jobCompleted(Avogadro::QtGui::BatchJob::BatchId,
                            Avogadro::QtGui::BatchJob::JobState)));

  for (MolRefList::const_iterator it = moleculeRefs.begin(),
       itEnd = moleculeRefs.end(); it != itEnd; ++it) {
    // Keep the GUI happy
    qApp->processEvents();

    mol = Molecule();
    if (AvogadroTools::createMolecule(*it, mol) && mol.atomCount() > 0) {
      BatchJob::BatchId id = batch->submitNextJob(mol);
      if (id != BatchJob::InvalidBatchId) {
        batch->registerMoleculeRef(id, *it);
        continue;
      }
    }
    qWarning() << "Error submitting job for molref" << it->id().c_str()
               << "No 3D structure information.";
  }

  return cleanup.take();
}

void BatchJobManager::jobCompleted(BatchJob::BatchId id,
                                   BatchJob::JobState state)
{
  BatchJobDecorator *batch = qobject_cast<BatchJobDecorator*>(sender());
  if (!batch)
    return;

  if (!m_batchJobs.contains(batch))
    return;

  // Clean up the batch job object if all jobs are finished.
  QScopedPointer<BatchJobDecorator> cleanup;
  if (batch->unfinishedJobCount() == 0) {
    m_batchJobs.removeOne(batch);
    cleanup.reset(batch);
  }

  if (state != BatchJob::Finished) {
    qDebug() << "Batch job id" << id << "for" << batch->description()
             << "did not finish successfully.";
    return;
  }

  MongoDatabase *db = MongoDatabase::instance();
  if (!db->connection()) {
    qDebug() << "mongodb not connected. Cannot add batch job result.";
    return;
  }

  MoleculeRef molRef = batch->moleculeRef(id);
  if (!molRef) {
    qDebug() << "Invalid MoleculeRef associated with batch job " << id;
    return;
  }

  MoleQueue::JobObject jobObject = batch->jobObject(id);
  QJsonObject calcOpts(batch->inputGeneratorOptions());
  calcOpts = calcOpts.value("options").toObject();

  // Extract some of the common parameters:
  QString description = batch->description();
  QString calcType = calcOpts.value("Calculation Type").toString();
  QString calcTheory = calcOpts.value("Theory").toString();
  QString calcBasis = calcOpts.value("Basis").toString();

  mongo::BSONObjBuilder calcBuilder;
  if (!calcTheory.isEmpty())
    calcBuilder << "theory" << calcTheory.toStdString();
  if (!calcBasis.isEmpty())
    calcBuilder << "basis" << calcBasis.toStdString();
  mongo::BSONObj calcObj = calcBuilder.obj();

  mongo::BSONObjBuilder docBuilder;
  docBuilder
      << "molecule" << BSON("$ref" << "molecules"
                            << "$id" << mongo::OID(molRef.id()))
      << "name" << (description.isEmpty() ? std::string("Batch job")
                                          : description.toStdString());
  if (!calcType.isEmpty())
    docBuilder << "type" << calcType.toStdString();

  if (calcObj.nFields() > 0)
    docBuilder << "calculation" << calcObj;

  // Store files in db.quantum.[files|chunks]
  mongo::GridFS gridfs(*db->connection(), db->databaseName(), "quantum");
  mongo::BSONArrayBuilder logFileBuilder;
  QDir outputDir(jobObject.value("outputDirectory").toString());
  if (outputDir.isReadable()) {
    foreach (const QFileInfo &info, outputDir.entryInfoList(QDir::Files)) {
      QFile file(info.absoluteFilePath());
      if (!file.open(QFile::ReadOnly) || !file.isReadable())
        continue;
      QByteArray fileData = file.readAll();
      mongo::BSONObj fileObj =
          gridfs.storeFile(fileData.constData(), fileData.size(),
                           info.fileName().toStdString());
      logFileBuilder << fileObj;
    }
  }

  mongo::BSONArray logFileArray = logFileBuilder.arr();
  if (logFileArray.nFields() > 0)
    docBuilder << "files" << BSON("log" << logFileArray);

  mongo::BSONObj docObj = docBuilder.obj();

  db->connection()->insert(db->quantumCollectionName(), docObj);
}

void BatchJobManager::refreshGenerators()
{
  m_scriptFiles.clear();

  // List of directories to check.
  QStringList dirs;

  // Installation prefix:
  dirs << QCoreApplication::applicationDirPath() +
          "/../lib/avogadro2/scripts/inputGenerators";
  dirs << QCoreApplication::applicationDirPath() +
          "/../lib64/avogadro2/scripts/inputGenerators";

  // Superbuild mongochem directory:
  dirs << QCoreApplication::applicationDirPath() +
          "/../../avogadrolibs/lib/avogadro2/scripts/inputGenerators";
  dirs << QCoreApplication::applicationDirPath() +
          "/../../avogadrolibs/lib64/avogadro2/scripts/inputGenerators";

  // Identify valid input generators:
  foreach (const QString &dirStr, dirs) {
    qDebug() << "Checking for input generator scripts in" << dirStr;
    QDir dir(dirStr);
    if (dir.exists() && dir.isReadable()) {
      foreach (const QFileInfo &file, dir.entryInfoList(QDir::Files |
                                                        QDir::NoDotAndDotDot)) {
        QString filePath = file.absoluteFilePath();
        QString displayName;
        if (queryProgramName(filePath, displayName)) {
          m_scriptFiles.insert(displayName, filePath);
        }
      }
    }
  }
}

bool BatchJobManager::queryProgramName(const QString &scriptFilePath,
                                             QString &name) const
{
  InputGenerator gen(scriptFilePath);
  name = gen.displayName();
  if (gen.hasErrors()) {
    name.clear();
    qWarning() << "Unable to retrieve program name for" << scriptFilePath
               << ":\n" << gen.errorList().join("\n");
    return false;
  }

  return true;
}

QAction* BatchJobManager::createAction(const QString &label,
                                             const QString &filePath) const
{
  QAction *action = new QAction(label, NULL);
  action->setData(filePath);
  return action;
}

} // namespace MongoChem
