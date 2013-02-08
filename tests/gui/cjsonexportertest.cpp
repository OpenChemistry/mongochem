/******************************************************************************

  This source file is part of the MoleQueue project.

  Copyright 2013 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#include "cjsonexporter.h"

#include "mongochemtestconfig.h"

#include <mongo/client/dbclient.h>

#include <QtTest>
#include <QtCore/QFile>


class CjsonExporterTest : public QObject
{
  Q_OBJECT
public:
  CjsonExporterTest()
    : QObject(NULL)
  {

  }

private slots:
  void toCjson();

};

void CjsonExporterTest::toCjson() {

  // Load test BSON data
  QString bsonFilePath = QString(MongoChem_TESTDATA_DIR) +
                           "/cjsonexporter/bsonobj.json";
  QFile bsonFile(bsonFilePath);
  if (!bsonFile.open(QFile::ReadOnly)) {
    qDebug() << "Cannot access data file" << bsonFilePath;
    return;
  }
  QByteArray bsonData = bsonFile.readAll();
  mongo::BSONObj obj(bsonData.constData());

  // Load the expected CJSON
  QString cjsonFilePath = QString(MongoChem_TESTDATA_DIR) +
                            "/cjsonexporter/cjson.json";
  QFile cjsonFile(cjsonFilePath);
  if (!cjsonFile.open(QFile::ReadOnly | QIODevice::Text)) {
    qDebug() << "Cannot access data file" << cjsonFilePath;
    return;
  }
  QByteArray cjsonData = cjsonFile.readAll();

  QCOMPARE(QByteArray(MongoChem::CjsonExporter::toCjson(obj).c_str()).trimmed(),
    cjsonData.trimmed());
}

QTEST_MAIN(CjsonExporterTest)

#include "cjsonexportertest.moc"
