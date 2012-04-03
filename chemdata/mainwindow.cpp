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

#include "mainwindow.h"

#include "mongomodel.h"
#include "detaildialog.h"
#include "substructurefiltermodel.h"

#include "ui_mainwindow.h"

#include <mongo/client/dbclient.h>

#include <QtGui/QSplitter>
#include <QtGui/QDialog>
#include <QtGui/QVBoxLayout>
#include <QtGui/QFileDialog>
#include <QtCore/QDebug>
#include <QtCore/QProcess>
#include <QtCore/QSettings>
#include <QtGui/QPainter>
#include <QtGui/QStyledItemDelegate>
#include <QtGui/QTextDocument>
#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QDockWidget>
#include <QtGui/QMessageBox>

#include <QVTKWidget.h>
#include <QVTKInteractor.h>
#include <vtkContextView.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkContextScene.h>
#include <vtkChartXY.h>
#include <vtkChartParallelCoordinates.h>
#include <vtkAxis.h>
#include <vtkFloatArray.h>
#include <vtkStringArray.h>
#include <vtkPlot.h>
#include <vtkTable.h>
#include <vtkAnnotationLink.h>
#include <vtkExtractSelectedRows.h>
#include <vtkEventQtSlotConnect.h>
#include <vtkSelection.h>

#include <chemkit/molecule.h>
#include <chemkit/moleculefile.h>

#include "graphdialog.h"
#include "quickquerywidget.h"
#include "serversettingsdialog.h"
#include "parallelcoordinatesdialog.h"
#include "plotmatrixdialog.h"
#include "moleculedetaildialog.h"

namespace {

class MolecularFormulaDelegate : public QStyledItemDelegate
{
public:
  MolecularFormulaDelegate(QObject *parent = 0)
    : QStyledItemDelegate(parent)
  {
  }

  QString toHtmlFormula(const QString &formula) const
  {
    QString htmlFormula;

    bool inNumber = false;

    foreach(const QChar &c, formula){
      if(c.isLetter() && inNumber){
        htmlFormula += "</sub>";
        inNumber = false;
      }
      else if(c.isNumber() && !inNumber){
        htmlFormula += "<sub>";
        inNumber = true;
      }

      htmlFormula += c;
    }

    if(inNumber){
      htmlFormula += "</sub>";
    }

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
    context.palette.setColor(QPalette::Text, painter->pen().color());

    painter->save();
    painter->translate(options.rect.x(), y);
    layout->draw(painter, context);
    painter->restore();
  }
};

} // end anonymous namespace

namespace ChemData {

MainWindow::MainWindow()
  : m_db(0),
    m_detail(0),
    m_model(0)
{
  m_ui = new Ui::MainWindow;
  m_ui->setupUi(this);

  // add query dock widget
  m_queryWidget = new QuickQueryWidget;
  connect(m_queryWidget, SIGNAL(queryClicked()), SLOT(runQuery()));
  QDockWidget *queryDockWidget = new QDockWidget("Query", this);
  m_ui->menu_View->addAction(queryDockWidget->toggleViewAction());
  queryDockWidget->setWidget(m_queryWidget);
  addDockWidget(Qt::TopDockWidgetArea, queryDockWidget);
  queryDockWidget->hide();

  connect(m_ui->actionGraphs, SIGNAL(activated()), SLOT(showGraphs()));
  connect(m_ui->actionPlotMatrix, SIGNAL(activated()), SLOT(showPlotMatrix()));
  connect(m_ui->actionParallelCoordinates, SIGNAL(activated()),
          this, SLOT(showParallelCoordinates()));
  connect(m_ui->actionServerSettings, SIGNAL(activated()), SLOT(showServerSettings()));
  connect(m_ui->actionAddNewData, SIGNAL(activated()), SLOT(addNewRecord()));
//  connect(m_ui->tableView, SIGNAL(doubleClicked(QModelIndex)),SLOT(showDetails(QModelIndex)));
  connect(m_ui->tableView, SIGNAL(doubleClicked(QModelIndex)), SLOT(showMoleculeDetailsDialog(QModelIndex)));
  connect(this, SIGNAL(connectionFailed()), this, SLOT(showServerSettings()), Qt::QueuedConnection);

  setupTable();
  connectToDatabase();
}

MainWindow::~MainWindow()
{
  delete m_db;
  delete m_model;
  m_model = 0;
  delete m_ui;
  m_ui = 0;
}

void MainWindow::connectToDatabase()
{
  // remove current database connection
  delete m_db;
  m_db = new mongo::DBClientConnection;

  // remove current model
  delete m_model;
  m_model = 0;
  m_ui->tableView->setModel(m_model);

  // connect to database
  QSettings settings;
  std::string host = settings.value("hostname").toString().toStdString();
  try {
    m_db->connect(host);
    std::cout << "Connected to: " << host << std::endl;
  }
  catch (mongo::DBException &e) {
    std::cerr << "Failed to connect to MongoDB at '" << host  << "': " << e.what() << std::endl;
    delete m_db;
    m_db = 0;
    emit connectionFailed();
    return;
  }

  m_model = new MongoModel(m_db, this);
  m_ui->tableView->setModel(m_model);
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

void MainWindow::showGraphs()
{
  GraphDialog dialog(this);

  dialog.exec();
}

void MainWindow::showPlotMatrix()
{
  PlotMatrixDialog dialog(this);

  dialog.exec();
}

void MainWindow::showParallelCoordinates()
{
  ParallelCoordinatesDialog dialog(this);

  dialog.exec();
}

void MainWindow::showDetails(const QModelIndex &index)
{
  if (!m_detail) {
    m_detail = new DetailDialog(this);
    m_detail->resize(600, 400);
  }
  m_detail->setActiveRecord(index);
  m_detail->show();
}

void MainWindow::showMoleculeDetailsDialog(const QModelIndex &index)
{
  MoleculeDetailDialog *dialog = new MoleculeDetailDialog(this);

  mongo::BSONObj *obj = static_cast<mongo::BSONObj *>(index.internalPointer());
  dialog->setMoleculeObject(obj);

  dialog->show();
}

void MainWindow::showServerSettings()
{
  ServerSettingsDialog dialog;

  if(dialog.exec() == QDialog::Accepted){
    QSettings settings;
    settings.setValue("hostname", dialog.host());
    settings.setValue("port", dialog.port());
    settings.setValue("collection", dialog.collection());

    // reload collection
    connectToDatabase();
  }
}

void MainWindow::addMoleculesFromFile(const QString &fileName)
{
  chemkit::MoleculeFile file(fileName.toStdString());

  bool ok = file.read();
  if(!ok){
    QMessageBox::warning(this,
                         "Error",
                         QString("Error reading file: %1").arg(file.errorString().c_str()));
  }

  QSettings settings;
  std::string collection =
      settings.value("collection").toString().toStdString();

  // index on inchikey
  m_db->ensureIndex(collection + ".molecules", BSON("inchikey" << 1), /* unique = */ true);

  // index on heavy atom count
  m_db->ensureIndex(collection + ".molecules", BSON("heavyAtomCount" << 1), /* unique = */ false);

  // index on value for the descriptors
  m_db->ensureIndex(collection + ".descriptors.vabc", BSON("value" << 1), /* unique = */ false);
  m_db->ensureIndex(collection + ".descriptors.xlogp3", BSON("value" << 1), /* unique = */ false);
  m_db->ensureIndex(collection + ".descriptors.mass", BSON("value" << 1), /* unique = */ false);
  m_db->ensureIndex(collection + ".descriptors.tpsa", BSON("value" << 1), /* unique = */ false);

  foreach(const boost::shared_ptr<chemkit::Molecule> &molecule, file.molecules()){
    std::string name = molecule->data("PUBCHEM_IUPAC_TRADITIONAL_NAME").toString();
    if(name.empty())
      name = molecule->name();

    std::string formula = molecule->formula();

    std::string inchi = molecule->data("PUBCHEM_IUPAC_INCHI").toString();
    if(inchi.empty())
      inchi = molecule->formula("inchi");

    std::string inchikey = molecule->data("PUBCHEM_IUPAC_INCHIKEY").toString();
    if(inchikey.empty())
      inchikey = molecule->formula("inchikey");

    double mass = molecule->data("PUBCHEM_MOLECULAR_WEIGHT").toDouble();
    if(mass == 0.0)
      mass = molecule->mass();

    int atomCount = molecule->atomCount();
    int heavyAtomCount = molecule->atomCount() - molecule->atomCount("H");

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
  if (!fileName.isEmpty()) {
    addMoleculesFromFile(fileName);
  }
}

void MainWindow::clearDatabase()
{
  QSettings settings;
  std::string collection =
      settings.value("collection").toString().toStdString();

  // drop the current molecules collection
  m_db->dropCollection(collection + ".molecules");
  m_db->dropCollection(collection + ".descriptors.vabc");
  m_db->dropCollection(collection + ".descriptors.xlogp3");
  m_db->dropCollection(collection + ".descriptors.mass");
  m_db->dropCollection(collection + ".descriptors.tpsa");
}

void MainWindow::selectionChanged()
{
  m_extract->Update();

  m_extract->Update();
  m_chart3->RecalculateBounds();
  m_vtkWidget3->update();

  m_vtkWidget->update();
  m_vtkWidget2->update();
}

void MainWindow::chartPointClicked(vtkObject *, unsigned long,
                                   void*, void *client_data2,
                                   vtkCommand*)
{
  vtkChartPlotData *plot = static_cast<vtkChartPlotData*>(client_data2);
  qDebug() << "Series Name:" << plot->SeriesName.c_str()
           << "Index:" << plot->Index;

  m_ui->tableView->selectRow(plot->Index);

/*  emit pointClicked(QString(plot->SeriesName.c_str()),
                    Vector2f(plot->Position.GetData()),
                    Vector2i(plot->ScreenPosition.GetData()),
                    plot->Index); */
}

void MainWindow::runQuery()
{
  // delete the old model if it is not the main model (e.g. it
  // is a filter model such as SubstructureFilterModel)
  if(m_ui->tableView->model() != m_model){
    m_ui->tableView->model()->deleteLater();
  }

  m_ui->tableView->setModel(0);

  // update the ui to show an empty table while the
  // query is performed
  qApp->processEvents();

  m_model->setQuery(m_queryWidget->query());

  if(m_queryWidget->field() == "Structure"){
    SubstructureFilterModel *model = new SubstructureFilterModel;
    model->setSmiles(m_queryWidget->value());
    model->setSourceModel(m_model);
    m_ui->tableView->setModel(model);
  }
  else{
    m_ui->tableView->setModel(m_model);
  }

  m_ui->tableView->resizeColumnsToContents();
}

}
