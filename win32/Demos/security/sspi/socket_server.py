# A sample socket server and client, based on the standard MS samples
# "Using SSPI with a Windows Sockets Client[/Server]"

import sys
import struct
import SocketServer
import win32api
import httplib

import win32security
import sspi, sspicon

# Send a simple "message" over a socket - send the number of bytes first,
# then the string.  Ditto for receive.
def _send_msg(s, m):
    s.send(struct.pack("i", len(m)))
    s.send(m)

def _get_msg(s):
    size_data = s.recv(struct.calcsize("i"))
    if not size_data:
        return None
    cb = struct.unpack("i", size_data)[0]
    return s.recv(cb)

class SSPISocketServer(SocketServer.TCPServer):
    def __init__(self, *args, **kw):
        SocketServer.TCPServer.__init__(self, *args, **kw)
        self.sa = sspi.ServerAuth("NTLM")

    def verify_request(self, sock, ca):
        # Do the sspi auth dance
        self.sa.reset()
        while 1:
            data = _get_msg(sock)
            if data is None:
                return False
            err, sec_buffer = self.sa.authorize(data)
            if err==0:
                break
            _send_msg(sock, sec_buffer[0].Buffer)
        return True

    def process_request(self, request, client_address):
        # An example using the connection once it is established.
        print "The server is running as user", win32api.GetUserName()
        self.sa.ctxt.ImpersonateSecurityContext()
        try:
            print "Having conversation with client as user", win32api.GetUserName()
            while 1:
                data = _get_msg(request)
                if not data:
                    break
                data = self.sa.decrypt(data)
                print "Client sent:", repr(data)
        finally:
            self.sa.ctxt.RevertSecurityContext()
        self.close_request(request)
        print "The server is back to user", win32api.GetUserName()

def serve():
    s = SSPISocketServer(("localhost", 8181), None)
    print "Running test server..."
    s.serve_forever()

def sspi_client():
    c = httplib.HTTPConnection("localhost", 8181)
    c.connect()
    # Do the auth dance.
    ca = sspi.ClientAuth("NTLM")
    data = None
    while 1:
        err, out_buf = ca.authorize(data)
        _send_msg(c.sock, out_buf[0].Buffer)
        if err==0:
            break
        data = _get_msg(c.sock)
    print "Auth dance complete - sending single encryted message"
    # Assume out data is sensitive - encrypt the message.
    _send_msg(c.sock, ca.encrypt("Hello"))
    c.sock.close()
    print "Client completed."

if __name__=='__main__':
    command = ""
    if len(sys.argv)>1: command = sys.argv[1]
    if command == "client":
        sspi_client()
    elif command == "server":
        serve()
    else:
        print "You must execute this with either 'client' or 'server'"
        print "Start an instance with 'server', then connect to it with 'client'"
        print
        print "Running either the client or server as a different user is"
        print "recommended. A command-line such as the following may be useful:"
        try:
            un = win32api.GetUserNameEx(win32api.NameSamCompatible)
        except win32api.error:
            # not in a domain
            un = win32api.GetUserName()
        print "runas /user:%s {path_to}\python.exe {path_to}\%s client|server" % (un, __file__)
