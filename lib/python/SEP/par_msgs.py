import SEP.sep_socket
import SEP.args
import SEP.opt_none
import SEP.pj_func
import os,sys,string,time,commands,signal,types,re,pwd
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.03.21"
                                                                                

class msg_obj:
  """A Class for storing messages"""
  def __init__(self,logfile):
    """Initialzize a message object 
    
       logfile - the file to store messages from the server

    """
    self.file=logfile
    self.msg_desc =SEP.util.temp_desc("msgs")
    self.started=1

  def clean(self):
    """Remove the message file"""
    SEP.spawn.run_wait("rm %s"%self.file,50)

  def init(self):
    """Initialize the message file"""
    SEP.spawn.run_wait("rm %s;touch %s"%(self.file,self.file),50)

  def read_delete(self):
     """Read the message file contents,clear the file, and return the messages"""
     SEP.util.lock_desc(self.msg_desc)
     lines,err=self.read_msgs()
     self.write_msgs()
     SEP.util.unlock_desc(self.msg_desc)
     return lines,err

  def read_msgs(self):
    """Read the messages"""
    try: f=open(self.file)
    except: SEP.util.err("trouble reading msg file %s"%self.file)
    lines=f.readlines()
    f.close()
    return lines,None

  def write_msgs(self,lines=None):
    """Write the message file"""
    try: f=open(self.file,"w")
    except: SEP.util.err("trouble opening msg file %s"%self.file)
    if lines:
      for line in lines: f.write("%s"%line)
    f.close()


  def action(self,msg):
     """Perform an action based on a message (msg)"""
     oper,jobid,machid,extra=SEP.pj_func.decode_action(msg)
     if  oper=="exit": 
       self.started=None
       return None
     SEP.util.lock_desc(self.msg_desc)
     lines,err=self.read_msgs()
     lines.append("%s\n"%msg)
     self.write_msgs(lines)
     SEP.util.unlock_desc(self.msg_desc)
     if err: return None
     return 1

class server_msg_obj(msg_obj,SEP.sep_socket.sep_server,SEP.opt_none.options):
  """A Class for storing messages and communicating with processes"""
  def __init__(self,logfile,nmax,device):
     """A class that handles messages from a socket connection

        device  - the device  on the master server (defaults to eth1)
        logfile - the file where the message stored
        nmax    - the maximum number of processes that can share a socket

        SEE parent classes for more options

     """
     msg_obj.__init__(self,logfile)
     self.nmax=nmax
     self.label_list=[]
     SEP.sep_socket.sep_server.__init__(self,device)
     SEP.opt_none.options(name="PARJOB")
     self.started=None

  def remove_use(self,mlabel):
    """Mark that the given machine label is no longer using this socket"""
    if self.label_list.count(mlabel)==1:
      del self.label_list[self.label_list.index(mlabel)]
    else: SEP.util.error("Internal error, deleting non-used mlabel")

  def add_use(self,mlabel,port_num):
    """Try to add the given process to this socket

       mlabel   -  the machine label to attempt to add
       port_num -  the port number to attempt to use 
       
       Returns stat, pid, next_port 
        
       stat     - 1 if it can be added otherwise None
       pid      - the process id for the forked process
       next_port- next free port

    """
    if len(self.label_list) < self.nmax:
      self.label_list.append(mlabel)
      if not self.started:
         self.started=1
         pid,next_port=self.run_server(port_num)
         if not pid: sys.exit(0)
         return 1,pid,next_port
      return  1,None,None
    return None,None,None

