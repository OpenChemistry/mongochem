#include <mongo/client/dbclient.h>

#include <QtCore>
#include <QtNetwork>

using namespace mongo;

int main(int argc, char *argv[])
{
  QCoreApplication app(argc, argv);

  // connect to mongo database
  DBClientConnection db;
  std::string host = "localhost";
  try {
    db.connect(host);
    std::cout << "Connected to: " << host << std::endl;
  }
  catch (DBException &e) {
    std::cerr << "Failed to connect to MongoDB: " << e.what() << std::endl;
  }

  auto_ptr<DBClientCursor> cursor = db.query("chem.molecules");

  QNetworkAccessManager networkManager;

  while(cursor->more()){
    BSONObj obj = cursor->next();

    if(!obj.getField("diagram").eoo()){
      continue;
    }

    std::string name = obj.getField("name").str();
    std::string inchi = obj.getField("inchi").str();
//    std::string inchikey = obj.getField("inchikey").str();

    // fetch image
    QString url = "http://cactus.nci.nih.gov/chemical/structure/" + QString::fromStdString(inchi) + "/image?format=png";

//    qDebug() << "name:" << name.c_str();
    qDebug() << "inchi:" << inchi.c_str();

    QNetworkReply *reply = networkManager.get(QNetworkRequest(QUrl(url)));
 
    QEventLoop loop;
    QObject::connect(reply, SIGNAL(readyRead()), &loop, SLOT(quit()));
    
    loop.exec();

    QByteArray image = reply->readAll();

//    if(image.contains("Page not found (404)")){
//      delete reply;
//      reply = 0;

//      qDebug() << "name:" << name.c_str();

//      // try with iupac name
//      url = "http://cactus.nci.nih.gov/chemical/structure/" + QString::fromStdString(name) + "/image?format=png";
//      QNetworkReply *reply = networkManager.get(QNetworkRequest(QUrl(url)));
//      QObject::connect(reply, SIGNAL(readyRead()), &loop, SLOT(quit()));
//      loop.exec();

//      image = reply->readAll();
//    }

    if(!image.contains("Page not found (404)")){
      BSONObjBuilder b;
      b.appendBinData("diagram", image.length(), mongo::BinDataGeneral, image.data());

      BSONObjBuilder updateSet;
      updateSet << "$set" << b.obj();

      qDebug() << "image size: " << image.size();

      db.update("chem.molecules",
                obj,
                updateSet.obj(),
                true /* upsert */);
    }
  
    delete reply;
  }

  return 0;
}

