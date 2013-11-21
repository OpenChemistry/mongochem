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

#ifndef MONGOCHEM_BATCHJOBMANAGER_H
#define MONGOCHEM_BATCHJOBMANAGER_H

#include <QtCore/QObject>

#include "batchjobdecorator.h" // for typedefs in slots

#include <QtGui/QAction>
#include <QtGui/QDialog>

#include <QtCore/QList>
#include <QtCore/QMultiMap>
#include <QtCore/QString>

namespace MongoChem {
class MongoModel;

/**
 * @brief The BatchJobManager singleton is used to manage computational
 * input file generators and batch jobs.
 */
class BatchJobManager : public QObject
{
  Q_OBJECT
public:
  /** Return the singleton instance. */
  static BatchJobManager &instance();

  /**
   * Create and return a list of QActions, one for each input generator script.
   * The return objects are unowned and must be managed by the caller.
   */
  QList<QAction*> createActions() const;

  /**
   * @brief Perform a batch calculation on all molecules with 3D structures in
   * model.molecules().
   *
   * Using the supplied @a action, which should be obtained from
   * createActions(), prompt the user to configure a computational job using the
   * appropriate input generator and then replicate the job for all molecules in
   * @a model. The jobs are then submitted to a running MoleQueue server.
   * @a windowParent is used for parenting any dialogs that are displayed.
   *
   * @note This only runs jobs on the currently loaded molecules in the
   * model, not the entire database or query result.
   */
  BatchJobDecorator* performBatchCalculation(QWidget *windowParent,
                                             const QAction &action,
                                             const MongoModel &model);

private slots:
  void jobCompleted(Avogadro::QtGui::BatchJob::BatchId id,
                    Avogadro::QtGui::BatchJob::JobState state);

private:
  explicit BatchJobManager(QObject *parent = 0);
  ~BatchJobManager();

  /** Rescan for input generator scripts. */
  void refreshGenerators();

  /**
   * Load the input generator at @a scriptFilePath and put its display name into
   * @a name. Return true on success or false if the script is invalid.
   */
  bool queryProgramName(const QString &scriptFilePath, QString &name) const;

  /**
   * Create and return a new heap-allocated QAction with the @a label specified.
   * Set the filePath of the corresponding input generator script as the
   * action's .data() member.
   */
  QAction *createAction(const QString &label, const QString &filePath) const;

  static BatchJobManager *m_instance;
  QMultiMap<QString, QString> m_scriptFiles;
  QList<BatchJobDecorator*> m_batchJobs;
};

} // namespace MongoChem

#endif // MONGOCHEM_BATCHJOBMANAGER_H
