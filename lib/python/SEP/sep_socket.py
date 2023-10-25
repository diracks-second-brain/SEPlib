import time,socket,os,sys,string,thread,re,syslog,time,commands
import SEP.util,SEP.spawn

__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2004.12.2"
                                                                                


def get_ip_address (interface):
    """Get the ip address associated with a given interface"""
    stat,out=commands.getstatusoutput("/sbin/ifconfig %s"%interface)
    if stat!=0: SEP.util.err("trouble running ifconfig on %s"%interface)
    ip_addr = re.search ("^.*inet addr:([0-9]{1,3}(\.[0-9]{1,3}){3})", out, re.MULTILINE)
    if not ip_addr: SEP.util.err("trouble with ifconfig output %s"%out)
    ip_addr = re.search ("([0-9]{1,3}(\.[0-9]{1,3}){3})", ip_addr.group (0))
    if not ip_addr: SEP.util.err("trouble with ifconfig output %s"%out)
    return ip_addr.group (0)



def check_port(ip,test_port):
   """Check to see if a port is free"""
   sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
   try: sock.connect((ip, int(test_port)))
   except:  return 1
   sock.close()
   return None

class sep_server:
  """A simple socket server"""
  def __init__(self,device="eth1"):
    """Initialize a sep server object

       device - (eth1) The device associated with the cluster (defaults to eth1)

    """
    self.master_ip=get_ip_address(device)


  
  def ip_server(self):
    """"Return ip_server for socket """
    return self.master_ip

  def port_number(self):
    """Return the port number for this server"""
    return self.port_num

  def action(self,msg):
    """How to handle a message, should be overwritten"""
    return 1
  def run_server(self,port_num):
    """Run the server"""

    sock = socket.socket( socket.AF_INET, socket.SOCK_STREAM)
    sys.stdout.flush()
    started=None
    while not started:
      try:
        sock.bind(('',int(port_num)))
        started=1
      except:
        port_num=port_num+1
        
    pid=os.fork()
    self.port_num=port_num
    if pid: return pid,port_num+1
    sock.listen(5)
    ret=1
    first=1
    while ret:    # Run until cancelled
      newsock, client_addr = sock.accept()
      ret=self.handleClient(newsock)
      if ret == None: return None,None
    return None,None

  def server_pars(self,pars):
    """Update a parameter database with the parameters needed to communicate with the server"""
    pars.add_param("sep.pid",os.getpid())
    pars.add_param("sep.master_port",self.port_number())
    pars.add_param("sep.master_ip",self.master_ip)
    return pars


  def handleClient(self,sock):
    """Handle a message from the client"""
    data = sock.recv(1024)
    sock.send("EnDiT")
    index=data.find("EnDiT");
    if index ==-1:  return 1
    msgit=data[0:index]
    sock.close()
    a= self.action(msgit)
    return a


                                                                                    

