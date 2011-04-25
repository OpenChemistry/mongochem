#include <iostream>
#include <mongo/client/dbclient.h>

using namespace mongo;
using std::cout;
using std::endl;

void run()
{
  DBClientConnection c;
  c.connect("localhost");
}

int main()
{
  try {
    run();
    cout << "connected OK" << endl;
  }
  catch (DBException &e) {
    cout << "caught " << e.what() << endl;
  }
}

