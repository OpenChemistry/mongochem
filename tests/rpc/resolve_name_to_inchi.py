
import unittest
import chemdatarpc

# the ResolveNameToInchiTest tests the "convert_molecule_identifier"
# RPC method for converting molecule names (e.g. 'ethanol') to InChI
# and InChIKeys
class ResolveNameToInchiTest(unittest.TestCase):
  @classmethod
  def setUpClass(self):
    self.conn = chemdatarpc.Connection()

  def test_ethanol(self):
    self.conn.send_json(
      {
        'jsonrpc' : '2.0',
        'method' : 'convert_molecule_identifier',
        'params' : {
          'identifier' : 'ethanol',
          'input_format' : 'name',
          'output_format' : 'inchi'
        }
      }
    )

    reply = self.conn.recv_json()

    self.assertEqual(reply["inchi"], "InChI=1S/C2H6O/c1-2-3/h3H,2H2,1H3")

if __name__ == '__main__':
  unittest.main()
