#include <QtGui/QApplication>

#include "mainwindow.h"

#include <iostream>
#include <mongo/client/dbclient.h>
#include <mongo/client/gridfs.h>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QFileInfo>
#include <QtCore/QFile>
#include <QtCore/QDebug>

using namespace mongo;
using std::cout;
using std::endl;

QString m_fileName = "/home/marcus/ssd/src/avogadro/testfiles/benzene.mold";
QFileInfo info(m_fileName);

int main(int argc, char *argv[])
{

  QCoreApplication::setOrganizationName("Kitware");
  QCoreApplication::setOrganizationDomain("kitware.com");
  QCoreApplication::setApplicationName("ChemData");
  QCoreApplication::setApplicationVersion("0.1.0");
  QApplication app(argc, argv);

  ChemData::MainWindow window;
  window.show();
  return app.exec();

  DBClientConnection c;
  try {
    c.connect("localhost");
    cout << "connected OK" << endl;
  }
  catch (DBException &e) {
    cout << "caught " << e.what() << endl;
    return 1;
  }

  QFile file("/home/marcus/SBIR_chem_data.csv");
  file.open(QIODevice::ReadOnly);
  file.readLine();
  while (!file.atEnd()) {
    QString tmp = file.readLine();
    QStringList cmp = tmp.split(",");
    if (cmp.size() == 5) {
      qDebug() << "String:" << cmp[0];
      BSONObj p = BSONObjBuilder().genOID()
                                  .append("CAS", cmp[0].trimmed().toStdString())
                                  .append("Set", cmp[1].trimmed().toStdString())
                                  .append("Observed", cmp[2].toDouble())
                                  .append("Predicted log Sw (MLR)", cmp[3].toDouble())
                                  .append("Predicted log Sw (RF)", cmp[4].toDouble())
                                  .obj();
      c.insert("chem.import", p);
    }
    else
      qDebug() << "Wrong size" << cmp.size() << tmp;
  }

  return 0;

  GridFS gfs(c, "tutorial", "fs");
  BSONObj ret = gfs.storeFile(m_fileName.toStdString(),
                              info.fileName().toStdString());
  BSONObj b = BSONObjBuilder().genOID()
                              .appendElements(ret)
            .append("fileInserted", "01")
            .obj();

  BSONObj p = BSONObjBuilder().genOID()
                               .append("name", "Joe")
             .append("age", 33)
             .obj();
  c.insert("tutorial.persons", p);
  c.insert("tutorial.persons", b);

  cout << "count: " << c.count("tutorial.persons") << endl;
  auto_ptr<DBClientCursor> cursor =
    c.query("tutorial.persons");//QUERY("age" << 33));
  while (cursor->more())
    cout << cursor->next().toString() << endl;
}

