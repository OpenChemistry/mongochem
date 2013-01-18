import os
import sys
import time
import unittest
import mongochemrpc

# This tests the "getChemicalJson" RPC method.
class GetChemicalJsonTest(unittest.TestCase):
  @classmethod
  def setUpClass(self):
    self.conn = mongochemrpc.Connection()

  @classmethod
  def tearDownClass(self):
    self.conn.close()

  def test_ethanol(self):
    self.conn.send_json(
      {
        'jsonrpc' : '2.0',
        'id' : 0,
        'method' : 'getChemicalJson',
        'params' : {
          'inchi' : "InChI=1S/C2H6O/c1-2-3/h3H,2H2,1H3"
        }
      }
    )

    reply = self.conn.recv_json()

    self.assertEqual(reply["result"], "ethanol")

if __name__ == '__main__':
  # start mongochem
  mongochemrpc.start_mongochem(sys.argv[1])

  # run the tests
  unittest.main(argv = [sys.argv[0]])
