import os
import sys
import time
import unittest
import mongochemrpc

# the ResolveNameToInchiTest tests the "convertMoleculeIdentifier"
# RPC method for converting molecule names (e.g. 'ethanol') to InChI
# and InChIKeys
class ResolveNameToInchiTest(unittest.TestCase):
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
        'method' : 'convertMoleculeIdentifier',
        'params' : {
          'identifier' : 'methanol',
          'inputFormat' : 'name',
          'outputFormat' : 'inchi'
        }
      }
    )

    reply = self.conn.recv_json()

    self.assertEqual(reply["result"], "InChI=1S/CH4O/c1-2/h2H,1H3")

if __name__ == '__main__':
  # start mongochem
  mongochemrpc.start_mongochem(sys.argv[1])

  # run the tests
  unittest.main(argv = [sys.argv[0]])
