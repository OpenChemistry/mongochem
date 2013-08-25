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

#include "importsdffiledialog.h"
#include "ui_importsdffiledialog.h"

#include <QtCore/QDebug>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QProgressDialog>

#include <chemkit/molecule.h>
#include <chemkit/moleculefile.h>

#include <mongochem/gui/mongodatabase.h>
#include <mongochem/gui/svggenerator.h>

ImportSdfFileDialog::ImportSdfFileDialog(QWidget *parent_)
  : AbstractImportDialog(parent_),
    ui(new Ui::ImportSdfFileDialog)
{
  ui->setupUi(this);

  connect(ui->fileNameButton, SIGNAL(clicked()),
          this, SLOT(openFile()));
  connect(ui->importButton, SIGNAL(clicked()),
          this, SLOT(import()));
  connect(ui->cancelButton, SIGNAL(clicked()),
          this, SLOT(reject()));

  m_progressDialog = new QProgressDialog(this);
  m_progressDialog->setMinimumDuration(0);
  m_progressDialog->hide();
}

ImportSdfFileDialog::~ImportSdfFileDialog()
{
  delete ui;
}

void ImportSdfFileDialog::setFileName(const QString &fileName_)
{
  if (fileName_ != m_fileName) {
    // set new file name
    m_fileName = fileName_;

    // update ui
    ui->fileNameLineEdit->setText(m_fileName);
  }
}

QString ImportSdfFileDialog::fileName() const
{
  return m_fileName;
}

void ImportSdfFileDialog::openFile()
{
  QString fileName_ =
    QFileDialog::getOpenFileName(this, "Open SDF File", QString(),
                                 tr("SDF Files (*.sdf *.sdf.gz *.sdf.bz2 "
                                               "*.mol *.mol.gz *.mol.bz2 "
                                               "*.mdl *.mdl.gz *.mdl.bz2)"));

  if (!fileName_.isEmpty())
    setFileName(fileName_);
}

void ImportSdfFileDialog::import()
{
  m_progressDialog->setLabelText("Importing Molecules");
  m_progressDialog->show();

  chemkit::MoleculeFile file(m_fileName.toStdString());

  MongoChem::MongoDatabase *db = MongoChem::MongoDatabase::instance();
  mongo::DBClientConnection *conn = db->connection();

  bool ok = file.read();
  if (!ok) {
    QMessageBox::warning(this,
                         "Error",
                         QString("Error reading file: %1")
                           .arg(file.errorString().c_str()));
    return;
  }

  std::string collection = db->moleculesCollectionName();

  m_progressDialog->setMaximum(file.moleculeCount());

  foreach (const boost::shared_ptr<chemkit::Molecule> &molecule,
           file.molecules()) {
    // stop importing if the user clicked cancel
    if (m_progressDialog->wasCanceled())
      break;

    std::string name =
      molecule->data("PUBCHEM_IUPAC_TRADITIONAL_NAME").toString();
    if (name.empty())
      name = molecule->name();

    std::string formula = molecule->formula();
    std::string inchi = molecule->data("PUBCHEM_IUPAC_INCHI").toString();
    std::string inchikey = molecule->data("PUBCHEM_IUPAC_INCHIKEY").toString();

    double mass = molecule->data("PUBCHEM_MOLECULAR_WEIGHT").toDouble();
    if (mass == 0.0)
      mass = molecule->mass();

    int atomCount = static_cast<int>(molecule->atomCount());
    int heavyAtomCount =
      static_cast<int>(molecule->atomCount() - molecule->atomCount("H"));

    MongoChem::MoleculeRef ref = db->findMoleculeFromInChI(inchi);
    if (!ref){
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
        conn->insert(collection, b.obj());

        ref = MongoChem::MoleculeRef(id.str());
    }

    // read descriptors
    double tpsa = molecule->data("PUBCHEM_CACTVS_TPSA").toDouble();
    double xlogp3 = molecule->data("PUBCHEM_XLOGP3_AA").toDouble();
    double vabc = molecule->descriptor("vabc").toDouble();

    // set descriptors
    db->setMoleculeProperty(ref, "descriptors.mass", mass);
    db->setMoleculeProperty(ref, "descriptors.tpsa", tpsa);
    db->setMoleculeProperty(ref, "descriptors.xlogp3", xlogp3);
    db->setMoleculeProperty(ref, "descriptors.vabc", vabc);

    // generate diagrams
    if (!inchi.empty()) {
      // create and setup svg generator
      MongoChem::SvgGenerator *svgGenerator =
        new MongoChem::SvgGenerator(this);
      svgGenerator->setInputData(inchi.c_str());
      svgGenerator->setInputFormat("inchi");

      // listen to finished signal
      connect(svgGenerator, SIGNAL(finished(int)),
              this, SLOT(moleculeDiagramReady(int)));

      // store generator so we can clean it up later
      m_svgGenerators.insert(svgGenerator);

      // start the generation process in the background
      svgGenerator->start();
    }

    m_progressDialog->setValue(m_progressDialog->value() + 1);
  }
  m_progressDialog->setValue(file.moleculeCount());

  m_progressDialog->setLabelText("Generating Diagrams");
  m_progressDialog->setValue(0);
}

void ImportSdfFileDialog::moleculeDiagramReady(int errorCode)
{
  if (m_progressDialog->wasCanceled()) {
    m_svgGenerators.clear();
    m_progressDialog->hide();
    accept();
  }

  MongoChem::SvgGenerator *svgGenerator =
    qobject_cast<MongoChem::SvgGenerator *>(sender());
  if (!svgGenerator)
    return;

  QByteArray svg = svgGenerator->svg();

  if (errorCode != 0 || svg.isEmpty()) {
    qDebug() << "error generating svg";
    return;
  }

  // get molecule data
  QByteArray identifier = svgGenerator->inputData();

  // get molecule ref
  MongoChem::MongoDatabase *db = MongoChem::MongoDatabase::instance();
  MongoChem::MoleculeRef molecule =
    db->findMoleculeFromIdentifier(identifier.constData(), "inchi");

  // set molecule diagram
  if (molecule) {
    db->connection()->update(db->moleculesCollectionName(),
                             QUERY("_id" << molecule.id()),
                             BSON("$set" << BSON("svg" << svg.constData())),
                             true,
                             true);

    QByteArray png = svgGenerator->png();
    if (!png.isEmpty()) {
      mongo::BSONObjBuilder b;
      b.appendBinData(
        "diagram", png.length(), mongo::BinDataGeneral, png.constData());

      db->connection()->update(db->moleculesCollectionName(),
                               QUERY("_id" << molecule.id()),
                               BSON("$set" << b.obj()),
                               true,
                               true);
    }
  }

  // remove and delete generator
  m_svgGenerators.remove(svgGenerator);
  svgGenerator->deleteLater();

  m_progressDialog->setValue(m_progressDialog->maximum() - m_svgGenerators.size());

  if (m_svgGenerators.isEmpty()) {
    m_progressDialog->hide();
    accept();
  }
}
