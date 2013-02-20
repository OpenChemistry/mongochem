/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2011-2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "mainwindow.h"

#include "mongomodel.h"
#include "substructurefiltermodel.h"

#include "ui_mainwindow.h"

#include <boost/make_shared.hpp>

#include <mongo/client/dbclient.h>

#include <QDebug>
#include <QDir>
#include <QString>
#include <QRegExp>
#include <QLibrary>
#include <QtGui/QFileDialog>
#include <QtCore/QSettings>
#include <QtGui/QPainter>
#include <QtGui/QStyledItemDelegate>
#include <QtGui/QTextDocument>
#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QDockWidget>
#include <QtGui/QMessageBox>

#include <vtkAnnotationLink.h>

#include <chemkit/molecule.h>
#include <chemkit/fingerprint.h>
#include <chemkit/moleculefile.h>
#include <chemkit/pluginmanager.h>

#include "abstractvtkchartwidget.h"
#include "abstractclusteringwidget.h"
#include "abstractimportdialog.h"
#include "quickquerywidget.h"
#include "serversettingsdialog.h"
#include "moleculedetaildialog.h"
#include "mongodatabase.h"
#include "selectionfiltermodel.h"

#include <mongochem/plugins/pluginmanager.h>

#ifdef QTTESTING
#include <pqTestUtility.h>
#include <pqEventObserver.h>
#include <pqEventSource.h>
#include <QXmlStreamReader>
#endif

namespace {

class MolecularFormulaDelegate : public QStyledItemDelegate
{
public:
  MolecularFormulaDelegate(QObject *parent_ = 0)
    : QStyledItemDelegate(parent_)
  {
  }

  QString toHtmlFormula(const QString &formula) const
  {
    QString htmlFormula;

    bool inNumber = false;

    foreach (const QChar &c, formula) {
      if (c.isLetter() && inNumber) {
        htmlFormula += "</sub>";
        inNumber = false;
      }
      else if (c.isNumber() && !inNumber) {
        htmlFormula += "<sub>";
        inNumber = true;
      }

      htmlFormula += c;
    }

    if (inNumber)
      htmlFormula += "</sub>";

    return htmlFormula;
  }

  virtual void paint(QPainter *painter,
                     const QStyleOptionViewItem &option,
                     const QModelIndex &index) const
  {
    QStyleOptionViewItemV4 options = option;
    initStyleOption(&options, index);

    QTextDocument doc;
    doc.setHtml(toHtmlFormula(options.text));

    QAbstractTextDocumentLayout *layout = doc.documentLayout();

    int height = qRound(layout->documentSize().height());
    int y = options.rect.y() + (options.rect.height() - height) / 2;

    QAbstractTextDocumentLayout::PaintContext context;

    // save painter state
    painter->save();

    // draw background
    options.text = "";
    QStyle *style = options.widget ? options.widget->style() : QApplication::style();
    style->drawControl(QStyle::CE_ItemViewItem, &options, painter, options.widget);

    // draw text
    QPalette::ColorGroup colorGroup =
      options.state & QStyle::State_Enabled ? QPalette::Normal : QPalette::Disabled;
    if (colorGroup == QPalette::Normal && !(options.state & QStyle::State_Active))
      colorGroup = QPalette::Inactive;

    if (options.state & QStyle::State_Selected)
      painter->setPen(options.palette.color(colorGroup, QPalette::HighlightedText));
    else
      painter->setPen(options.palette.color(colorGroup, QPalette::Text));

    context.palette.setColor(QPalette::Text, painter->pen().color());

    painter->translate(options.rect.x(), y);
    layout->draw(painter, context);

    // restore painter state
    painter->restore();
  }
};

#ifdef QTTESTING
class XMLEventObserver : public pqEventObserver
{
  QXmlStreamWriter* XMLStream;
  QString XMLString;

public:
  XMLEventObserver(QObject* p) : pqEventObserver(p)
  {
    this->XMLStream = NULL;
  }
  ~XMLEventObserver()
  {
    delete this->XMLStream;
  }

protected:
  virtual void setStream(QTextStream* stream)
  {
    if (this->XMLStream) {
      this->XMLStream->writeEndElement();
      this->XMLStream->writeEndDocument();
      delete this->XMLStream;
      this->XMLStream = NULL;
    }
    if (this->Stream)
      *this->Stream << this->XMLString;

    this->XMLString = QString();
    pqEventObserver::setStream(stream);
    if (this->Stream) {
      this->XMLStream = new QXmlStreamWriter(&this->XMLString);
      this->XMLStream->setAutoFormatting(true);
      this->XMLStream->writeStartDocument();
      this->XMLStream->writeStartElement("events");
    }
  }

  virtual void onRecordEvent(const QString& widget, const QString& command,
                             const QString& arguments)
  {
    if(this->XMLStream) {
      this->XMLStream->writeStartElement("event");
      this->XMLStream->writeAttribute("widget", widget);
      this->XMLStream->writeAttribute("command", command);
      this->XMLStream->writeAttribute("arguments", arguments);
      this->XMLStream->writeEndElement();
    }
  }
};

class XMLEventSource : public pqEventSource
{
  typedef pqEventSource Superclass;
  QXmlStreamReader *XMLStream;

public:
  XMLEventSource(QObject* p): Superclass(p) { this->XMLStream = NULL;}
  ~XMLEventSource() { delete this->XMLStream; }

protected:
  virtual void setContent(const QString& xmlfilename)
  {
    delete this->XMLStream;
    this->XMLStream = NULL;

    QFile xml(xmlfilename);
    if (!xml.open(QIODevice::ReadOnly)) {
      qDebug() << "Failed to load " << xmlfilename;
      return;
    }
    QByteArray data = xml.readAll();
    this->XMLStream = new QXmlStreamReader(data);
  }

  int getNextEvent(QString& widget, QString& command, QString& arguments)
  {
    if (this->XMLStream->atEnd())
      return DONE;
    while (!this->XMLStream->atEnd()) {
      QXmlStreamReader::TokenType token = this->XMLStream->readNext();
      if (token == QXmlStreamReader::StartElement) {
        if (this->XMLStream->name() == "event")
          break;
      }
    }
    if (this->XMLStream->atEnd())
      return DONE;

    widget = this->XMLStream->attributes().value("widget").toString();
    command = this->XMLStream->attributes().value("command").toString();
    arguments = this->XMLStream->attributes().value("arguments").toString();
    return SUCCESS;
  }
};
#endif // QTTESTING

} // end anonymous namespace

namespace MongoChem {

template<class Factory>
inline QMap<QString, Factory *>
MainWindow::loadFactoryPlugins(QMenu *menu, const char *slot)
{
  MongoChem::PluginManager *manager = MongoChem::PluginManager::instance();

  QMap<QString, Factory *> map;

  foreach (Factory *factory, manager->pluginFactories<Factory>()) {
    // print debugging message
    qDebug() << "Loaded a factory with ID" << factory->identifier();

    // get the factory's name
    const QString &name = factory->identifier();

    // ensure that the name is unique
    if (map.contains(name)) {
      qDebug() << "Duplicate plugin found:" << name
               << "\nDiscarding duplicate plugins - IDs must be unique.";
      continue;
    }

    // add name and factory to the map
    map[name] = factory;

    // create a new menu action for this plugin
    QAction *action = new QAction(name, this);
    action->setData(name);
    connect(action, SIGNAL(triggered()), this, slot);
    menu->addAction(action);
  }

  return map;
}

MainWindow::MainWindow()
  : m_db(0),
    m_model(0)
{
  m_ui = new Ui::MainWindow;
  m_ui->setupUi(this);

  QIcon icon(":/icons/mongochem.png");
  setWindowIcon(icon);

  // add query dock widget
  m_queryWidget = new QuickQueryWidget;
  connect(m_queryWidget, SIGNAL(queryClicked()), SLOT(runQuery()));
  QDockWidget *queryDockWidget = new QDockWidget("Query", this);
  m_ui->menu_View->addAction(queryDockWidget->toggleViewAction());
  queryDockWidget->setWidget(m_queryWidget);
  addDockWidget(Qt::TopDockWidgetArea, queryDockWidget);
  queryDockWidget->hide();

#ifdef QTTESTING
  QMenu *menu = menuBar()->addMenu(tr("&Testing"));
  QAction *actionRecord = new QAction(this);
  actionRecord->setText(tr("Record test..."));
  menu->addAction(actionRecord);
  QAction *actionPlay = new QAction(this);
  actionPlay->setText(tr("Play test..."));
  menu->addAction(actionPlay);

  connect(actionRecord, SIGNAL(triggered()), SLOT(recordTest()));
  connect(actionPlay, SIGNAL(triggered()), SLOT(playTest()));

  m_testUtility = new pqTestUtility(this);
  m_testUtility->addEventObserver("xml", new XMLEventObserver(this));
  m_testUtility->addEventSource("xml", new XMLEventSource(this));
#endif

  connect(m_ui->actionServerSettings, SIGNAL(activated()), SLOT(showServerSettings()));
  connect(m_ui->actionAddNewData, SIGNAL(activated()), SLOT(addNewRecord()));
  connect(m_ui->tableView, SIGNAL(showMoleculeDetails(MongoChem::MoleculeRef)),
          this, SLOT(showMoleculeDetailsDialog(MongoChem::MoleculeRef)));
  connect(m_ui->tableView, SIGNAL(showSimilarMolecules(MongoChem::MoleculeRef)),
          this, SLOT(showSimilarMolecules(MongoChem::MoleculeRef)));
  connect(this, SIGNAL(connectionFailed()), this, SLOT(showServerSettings()), Qt::QueuedConnection);

  connect(m_ui->actionCalculateFingerprints, SIGNAL(activated()),
          this, SLOT(calculateAndStoreFingerprints()));
  connect(m_ui->actionShowSelectedMolecules, SIGNAL(toggled(bool)),
          this, SLOT(setShowSelectedMolecules(bool)));

  // Load the MongoChem plugins.
  MongoChem::PluginManager *manager = MongoChem::PluginManager::instance();
  manager->load();

  m_charts =
    loadFactoryPlugins<AbstractVtkChartWidgetFactory>(
      m_ui->menuChart,
      SLOT(showChartWidget()));

  m_clustering =
    loadFactoryPlugins<AbstractClusteringWidgetFactory>(
      m_ui->menuClustering,
      SLOT(showClusteringWidget()));

  m_importers =
    loadFactoryPlugins<AbstractImportDialogFactory>(
      m_ui->menuImport,
      SLOT(showImportDialog()));

  // setup annotation link
  m_annotationEventConnector->Connect(m_annotationLink.GetPointer(),
                                      vtkCommand::AnnotationChangedEvent,
                                      this,
                                      SLOT(updateSelectionFilterModel()));

  setupTable();
  connectToDatabase();
}

MainWindow::~MainWindow()
{
  delete m_model;
  m_model = 0;
  delete m_ui;
  m_ui = 0;

  qDeleteAll(m_charts.values());
  qDeleteAll(m_clustering.values());
  qDeleteAll(m_importers.values());
}

void MainWindow::connectToDatabase()
{
  // remove current model
  delete m_model;
  m_model = 0;
  m_ui->tableView->setModel(m_model);

  // disconnect the current mongodatabase instance ( after the cursors have
  // been cleaned up ).
  if (m_db)
    MongoDatabase::instance()->disconnect();

  // connect to database
  m_db = MongoDatabase::instance()->connection();
  if (!m_db) {
    emit connectionFailed();
    return;
  }

  // setup model
  m_model = new MongoModel(m_db, this);
  m_ui->tableView->setModel(m_model);
  m_ui->tableView->resizeColumnsToContents();
}

void MainWindow::setupTable()
{
  m_ui->tableView->setAlternatingRowColors(true);
  m_ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

  m_ui->tableView->setSelectionMode(QAbstractItemView::ContiguousSelection);

  m_ui->tableView->setSortingEnabled(false);
  m_ui->tableView->resizeColumnsToContents();
  m_ui->tableView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
  m_ui->tableView->resizeRowsToContents();

  MolecularFormulaDelegate *formulaDelegate = new MolecularFormulaDelegate(this);
  m_ui->tableView->setItemDelegateForColumn(2, formulaDelegate);
}

void MainWindow::showMoleculeDetailsDialog(MongoChem::MoleculeRef ref)
{
  if (ref.isValid()) {
    MoleculeDetailDialog *dialog = new MoleculeDetailDialog(this);
    dialog->setMolecule(ref);
    dialog->show();
  }
  else {
    QMessageBox::critical(this,
                          "Error",
                          "Failed to find molecule from index.");
  }
}

void MainWindow::showServerSettings()
{
  ServerSettingsDialog dialog;

  if (dialog.exec() == QDialog::Accepted) {
    QSettings settings;
    settings.setValue("hostname", dialog.host());
    settings.setValue("port", dialog.port());
    settings.setValue("collection", dialog.collection());
    settings.setValue("user", dialog.userName());

    // reload collection
    connectToDatabase();
  }
}

void MainWindow::addMoleculesFromFile(const QString &fileName)
{
  chemkit::MoleculeFile file(fileName.toStdString());

  bool ok = file.read();
  if (!ok) {
    QMessageBox::warning(this,
                         "Error",
                         QString("Error reading file: %1").arg(file.errorString().c_str()));
  }

  QSettings settings;
  std::string collection =
      settings.value("collection").toString().toStdString();

  // index on inchikey
  m_db->ensureIndex(collection + ".molecules", BSON("inchikey" << 1),
                    /* unique = */ true);

  // index on heavy atom count
  m_db->ensureIndex(collection + ".molecules", BSON("heavyAtomCount" << 1),
                    /* unique = */ false);

  // index on value for the descriptors
  m_db->ensureIndex(collection + ".descriptors.vabc", BSON("value" << 1),
                    /* unique = */ false);
  m_db->ensureIndex(collection + ".descriptors.xlogp3", BSON("value" << 1),
                    /* unique = */ false);
  m_db->ensureIndex(collection + ".descriptors.mass", BSON("value" << 1),
                    /* unique = */ false);
  m_db->ensureIndex(collection + ".descriptors.tpsa", BSON("value" << 1),
                    /* unique = */ false);

  foreach (const boost::shared_ptr<chemkit::Molecule> &molecule,
           file.molecules()) {
    std::string name = molecule->data("PUBCHEM_IUPAC_TRADITIONAL_NAME").toString();
    if (name.empty())
      name = molecule->name();

    std::string formula = molecule->formula();

    std::string inchi = molecule->data("PUBCHEM_IUPAC_INCHI").toString();
    if (inchi.empty())
      inchi = molecule->formula("inchi");

    std::string inchikey = molecule->data("PUBCHEM_IUPAC_INCHIKEY").toString();
    if (inchikey.empty())
      inchikey = molecule->formula("inchikey");

    double mass = molecule->data("PUBCHEM_MOLECULAR_WEIGHT").toDouble();
    if (mass == 0.0)
      mass = molecule->mass();

    int atomCount = static_cast<int>(molecule->atomCount());
    int heavyAtomCount =
      static_cast<int>(molecule->atomCount() - molecule->atomCount("H"));

    mongo::OID id = mongo::OID::gen();

    mongo::BSONObjBuilder b;
    b << "_id" << id;
    b << "name" << name;
    b << "formula" << formula;
    b << "inchi" << inchi;
    b << "inchikey" << inchikey;
    b << "mass" << mass;
    b << "atomCount" << atomCount;
    b << "heavyAtomCount" << heavyAtomCount;

    // add molecule
    m_db->insert(collection + ".molecules", b.obj());

    // add descriptors
    m_db->update(collection + ".descriptors.mass",
                QUERY("id" << id),
                BSON("$set" << BSON("value" << mass)),
                true);

    double tpsa = molecule->data("PUBCHEM_CACTVS_TPSA").toDouble();
    m_db->update(collection + ".descriptors.tpsa",
                QUERY("id" << id),
                BSON("$set" << BSON("value" << tpsa)),
                true);

    double xlogp3 = molecule->data("PUBCHEM_XLOGP3_AA").toDouble();
    m_db->update(collection + ".descriptors.xlogp3",
                QUERY("id" << id),
                BSON("$set" << BSON("value" << xlogp3)),
                true);

    double vabc = molecule->descriptor("vabc").toDouble();
    m_db->update(collection + ".descriptors.vabc",
                QUERY("id" << id),
                BSON("$set" << BSON("value" << vabc)),
                true);
  }

  // refresh model
  m_ui->tableView->setModel(0);
  delete m_model;
  m_model = new MongoModel(m_db);
  m_ui->tableView->setModel(m_model);
  m_ui->tableView->resizeColumnsToContents();
  m_ui->tableView->setColumnWidth(0, 250);
}

void MainWindow::addNewRecord()
{
  QString fileName = QFileDialog::getOpenFileName(this, "Output file to store");
  if (!fileName.isEmpty())
    addMoleculesFromFile(fileName);
}

void MainWindow::clearDatabase()
{
  QSettings settings;
  std::string collection =
      settings.value("collection").toString().toStdString();

  // Drop the current molecules collection.
  m_db->dropCollection(collection + ".molecules");
  m_db->dropCollection(collection + ".descriptors.vabc");
  m_db->dropCollection(collection + ".descriptors.xlogp3");
  m_db->dropCollection(collection + ".descriptors.mass");
  m_db->dropCollection(collection + ".descriptors.tpsa");
}

void MainWindow::runQuery()
{
  // Delete the old model if it is not the main model (e.g. it is a filter model
  // such as SubstructureFilterModel).
  if (m_ui->tableView->model() != m_model)
    m_ui->tableView->model()->deleteLater();

  m_ui->tableView->setModel(0);

  // Update the UI to show an empty table while the query is performed.
  qApp->processEvents();

  m_model->setQuery(m_queryWidget->query());

  if (m_queryWidget->field() == "Structure") {
    SubstructureFilterModel *model = new SubstructureFilterModel;
    model->setSmiles(m_queryWidget->value());
    model->setSourceModel(m_model);
    m_ui->tableView->setModel(model);
  }
  else {
    m_ui->tableView->setModel(m_model);
  }

  m_ui->tableView->resizeColumnsToContents();
}

void MainWindow::showSimilarMolecules(MoleculeRef ref, size_t count)
{
  // Update the UI to show an empty table while the query is performed.
  m_ui->tableView->setModel(0);
  qApp->processEvents();

  // create fp2 fingerprint
  boost::scoped_ptr<chemkit::Fingerprint>
      fp2(chemkit::Fingerprint::create("fp2"));
  if (!fp2) {
    qDebug() << "Failed to create FP2 fingerprint.";
    return;
  }

  // get access to the database
  MongoDatabase *db = MongoDatabase::instance();

  // calculate fingerprint for the input molecule
  boost::shared_ptr<chemkit::Molecule> molecule = db->createMolecule(ref);
  chemkit::Bitset fingerprint = fp2->value(molecule.get());

  // calculate tanimoto similarity value for each molecule
  std::map<float, MoleculeRef> sorted;
  std::vector<MoleculeRef> refs = m_model->molecules();
  for (size_t i = 0; i < refs.size(); ++i) {
    float similarity = 0;
    mongo::BSONObj obj = db->fetchMolecule(refs[i]);
    mongo::BSONElement element = obj.getField("fp2_fingerprint");
    if (element.ok()) {
      // there is already a fingerprint stored for the molecule so
      // load and use that for calculating the similarity value
      int len = 0;
      const char *binData = element.binData(len);
      std::vector<size_t> binDataBlockVector(fingerprint.num_blocks());

      memcpy(&binDataBlockVector[0],
             binData,
             binDataBlockVector.size() * sizeof(size_t));

      chemkit::Bitset fingerprintValue(binDataBlockVector.begin(),
                                       binDataBlockVector.end());

      // ensure that the fingerprints are the same size. this is necessary
      // because different platforms may have different padding for the bits.
      fingerprintValue.resize(fingerprint.size());

      similarity =
        static_cast<float>(
          chemkit::Fingerprint::tanimotoCoefficient(fingerprint,
                                                    fingerprintValue));
    }
    else {
      // there is not a fingerprint calculated for the molecule so
      // create the molecule and calculate the fingerprint directly
      molecule = db->createMolecule(refs[i]);

      similarity =
        static_cast<float>(
          chemkit::Fingerprint::tanimotoCoefficient(fingerprint,
                                                    fp2->value(molecule.get())));
    }

    sorted[similarity] = refs[i];
  }

  // Clamp number of output molecules to the number of input molecules.
  count = std::min(count, refs.size());

  // Build a vector of refs composed of the most similar molecules.
  std::vector<MoleculeRef> similarMolecules(count);

  std::map<float, MoleculeRef>::const_reverse_iterator iter = sorted.rbegin();
  for (size_t i = 0; i < count; ++i)
    similarMolecules[i] = iter++->second;

  // set the similar molecules to show in the model
  m_model->setMolecules(similarMolecules);

  // update the view for the updated model
  m_ui->tableView->setModel(m_model);
  m_ui->tableView->resizeColumnsToContents();
}

bool MainWindow::calculateAndStoreFingerprints(const std::string &name)
{
  // create the fingerprint
  boost::scoped_ptr<chemkit::Fingerprint>
    fingerprint(chemkit::Fingerprint::create(name));
  if (!fingerprint)
    return false;

  // get access to the database
  MongoDatabase *db = MongoDatabase::instance();

  // get a list of the current molecules
  std::vector<MoleculeRef> refs = m_model->molecules();

  for (size_t i = 0; i < refs.size(); ++i) {
    // get the molecule from the ref
    MoleculeRef ref = refs[i];
    boost::shared_ptr<chemkit::Molecule> molecule = db->createMolecule(ref);

    // calculate the fingerprint value and store it in a vector
    std::vector<size_t> value;
    boost::to_block_range(fingerprint->value(molecule.get()),
                          std::back_inserter(value));

    // store the fingerprint for the molecule in the database
    mongo::BSONObjBuilder b;
    b.appendBinData(name + "_fingerprint",
                    static_cast<int>(value.size() * sizeof(size_t)),
                    mongo::BinDataGeneral,
                    reinterpret_cast<char *>(&value[0]));
    mongo::BSONObjBuilder updateSet;
    updateSet << "$set" << b.obj();
    m_db->update("chem.molecules", QUERY("_id" << ref.id()), updateSet.obj());
  }

  return true;
}

void MainWindow::setShowSelectedMolecules(bool enabled)
{
  // delete the old model if it is not the main model (e.g. it
  // is a filter model such as SelectionFilterModel)
  if (m_ui->tableView->model() != m_model)
    m_ui->tableView->model()->deleteLater();
  m_ui->tableView->setModel(m_model);

  // add a selection filter model if enabled
  if (enabled) {
    SelectionFilterModel *filterModel = new SelectionFilterModel(this);
    filterModel->setSourceModel(m_model);
    filterModel->setSelection(m_annotationLink->GetCurrentSelection());
    m_ui->tableView->setModel(filterModel);
    m_ui->tableView->resizeColumnsToContents();
  }
}

void MainWindow::updateSelectionFilterModel()
{
  // update selection filter model
  setShowSelectedMolecules(m_ui->actionShowSelectedMolecules->isChecked());
}

void MainWindow::showChartWidget()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if (!action)
    return;

  QString name = action->data().toString();

  // Show the chart widget.
  AbstractVtkChartWidgetFactory *factory = m_charts[name];
  AbstractVtkChartWidget *widget = factory->createInstance();
  if (widget) {
    widget->setParent(this);
    widget->setWindowFlags(Qt::Dialog);
    widget->setAttribute(Qt::WA_DeleteOnClose);
    widget->setSelectionLink(m_annotationLink.GetPointer());
    widget->show();
  }
}

void MainWindow::showClusteringWidget()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if (!action)
    return;

  QString name = action->data().toString();
  // Show the clustering widget.
  AbstractClusteringWidgetFactory *factory = m_clustering[name];
  AbstractClusteringWidget *widget = factory->createInstance();
  if (widget) {
    widget->setParent(this);
    widget->setWindowFlags(Qt::Dialog);
    widget->setAttribute(Qt::WA_DeleteOnClose);
    connect(widget, SIGNAL(moleculeDoubleClicked(MongoChem::MoleculeRef)),
            this, SLOT(showMoleculeDetailsDialog(MongoChem::MoleculeRef)));
    widget->show();
    qApp->processEvents();
    widget->setMolecules(m_model->molecules());
  }
}

void MainWindow::showImportDialog()
{
  QAction *action = qobject_cast<QAction *>(sender());
  if (!action)
    return;

  const QString &name = action->data().toString();

  AbstractImportDialog *dialog = m_importerInstances.value(name, 0);
  if (dialog) {
    // reuse existing instance
    dialog->show();
  }
  else {
    // create new dialog instance
    AbstractImportDialogFactory *factory = m_importers[name];
    dialog = factory->createInstance();

    if (dialog) {
      // set parent
      dialog->setParent(this, dialog->windowFlags());

      // store new instance
      m_importerInstances[name] = dialog;

      // show new dialog
      dialog->exec();

      // update table
      runQuery();
    }
  }
}

#ifdef QTTESTING
void MainWindow::recordTest()
{
  QString fileName =
    QFileDialog::getSaveFileName(this,
                                 tr("Test file name"),
                                 QString(),
                                 tr("XML Files (*.xml)"));

  if (!fileName.isEmpty())
    m_testUtility->recordTests(fileName);
}

void MainWindow::playTest()
{
  QString fileName =
    QFileDialog::getOpenFileName(this,
                                 tr("Test file name"),
                                 QString(),
                                 tr("XML Files (*.xml)"));

  if (!fileName.isEmpty())
    playTest(fileName);
}

void MainWindow::playTest(const QString &fileName)
{
  qDebug() << "Playing test: " << fileName;

  m_testUtility->playTests(fileName);

  // for now we always close the application after completing a test. this
  // can be changed later if we want to support running multiple tests.
  qApp->exit();
}

void MainWindow::playTestLater(const QString &fileName)
{
  QMetaObject::invokeMethod(this,
                            "playTest",
                            Qt::QueuedConnection,
                            Q_ARG(QString, fileName));
}
#endif

} // end MongoChem namespace
