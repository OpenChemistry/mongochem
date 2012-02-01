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

#include "MongoModel.h"
#include "DetailDialog.h"
#include "substructurefiltermodel.h"

#include "ui_mainwindow.h"

#include <QtGui/QSplitter>
#include <QtGui/QDialog>
#include <QtGui/QVBoxLayout>
#include <QtGui/QFileDialog>
#include <QtCore/QDebug>
#include <QtCore/QProcess>
#include <QtGui/QPainter>
#include <QtGui/QStyledItemDelegate>
#include <QtGui/QTextDocument>
#include <QtGui/QAbstractTextDocumentLayout>
#include <QtGui/QDockWidget>

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

#include "graphdialog.h"
#include "quickquerywidget.h"

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

MainWindow::MainWindow() : m_detail(0)
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

  // connect to database
  std::string host = "localhost";
  try {
    m_db.connect(host);
    std::cout << "Connected to: " << host << std::endl;
  }
  catch (mongo::DBException &e) {
    std::cerr << "Failed to connect to MongoDB: " << e.what() << std::endl;
  }

  setupTable();

  connect(m_ui->actionGraphs, SIGNAL(activated()), SLOT(showGraphs()));
  connect(m_ui->actionAddNewData, SIGNAL(activated()), SLOT(addNewRecord()));
  connect(m_ui->tableView, SIGNAL(doubleClicked(QModelIndex)),SLOT(showDetails(QModelIndex)));
}

MainWindow::~MainWindow()
{
  delete m_model;
  m_model = 0;
  delete m_ui;
  m_ui = 0;
}

void MainWindow::setupTable()
{
  m_model = new MongoModel(&m_db, this);
  m_ui->tableView->setModel(m_model);

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

void MainWindow::showDetails(const QModelIndex &index)
{
  if (!m_detail) {
    m_detail = new DetailDialog(this);
    m_detail->resize(600, 400);
  }
  m_detail->setActiveRecord(index);
  m_detail->show();
}

void MainWindow::addNewRecord()
{
  QString fileName = QFileDialog::getOpenFileName(this, "Output file to store");
  if (!fileName.isEmpty()) {
//    m_model->addOutputFile(fileName);
  }
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
