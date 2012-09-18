import json
import socket
import struct
import StringIO

class Connection:
  def __init__(self, name = "chemdata"):
    # create socket
    self.sock = socket.socket(socket.AF_UNIX,
                              socket.SOCK_STREAM)

    # connect
    self.sock.connect("/tmp/" + name)

  def send_json(self, obj):
    self.send_message(json.dumps(obj))

  def send_message(self, msg):
    sz = len(msg)
    hdr = struct.pack('>I', sz)
    pkt = hdr + msg
    self.sock.send(pkt)

  def recv_message(self, size = 1024):
    pkt = self.sock.recv(size)

    return pkt[4:]

  def recv_json(self):
    msg = self.recv_message()

    try:
      return json.loads(msg)
    except Exception as e:
      print 'error: ' + str(e)
      return {}

  def close(self):
    self.sock.close()
