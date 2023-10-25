import SocketServer,  sys
import re,commands
import SEP.display
from threading import Thread

my_unix_command = ['bc']
HOST = 'localhost'
PORT = 9876

g=re.compile("open:(\S+):(\S+)")
base_dir=""


class SingleTCPHandler(SocketServer.BaseRequestHandler):
    "One instance per connection.  Override handle(self) to customize action."
    def handle(self):
        # self.request is the client connection
        data = self.request.recv(1024)  # clip input at 1Kb
        fnd=g.search(data)
        if fnd:
          SEP.display.display("%s/%s"%(base_dir,fnd.group(1)),fnd.group(2));
        self.request.close()

class SimpleServer(SocketServer.ThreadingMixIn, SocketServer.TCPServer):
    # Ctrl-C will cleanly kill all spawned threads
    daemon_threads = True
    # much faster rebinding
    allow_reuse_address = True

    def __init__(self, server_address, RequestHandlerClass):
        SocketServer.TCPServer.__init__(self, server_address, RequestHandlerClass)

