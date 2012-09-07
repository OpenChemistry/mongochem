
import unittest
import chemdatarpc

# This tests the "get_chemical_json" RPC method.
class GetChemicalJsonTest(unittest.TestCase):
  @classmethod
  def setUpClass(self):
    self.conn = chemdatarpc.Connection()

  def test_ethanol(self):
    self.conn.send_json(
      {
        'jsonrpc' : '2.0',
        'method' : 'get_chemical_json',
        'params' : {
          'inchi' : "InChI=1S/C2H6O/c1-2-3/h3H,2H2,1H3"
        }
      }
    )

    reply = self.conn.recv_json()

    self.assertEqual(reply["name"], "ethanol")

if __name__ == '__main__':
  unittest.main()
