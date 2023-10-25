import commands
import SEP.util
import SEP.log
import os,string,fcntl,re,types
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.3.17"


class status:
  """A class for storing the status of an object

     Status can be read/written to/from a file in a process safe mechanism

     Multiple routines for interacting with this database

  """

  def __init__(self,file,job_list,order=None,load=None):
     """Initialize a status object

        file to write the status to
        job_list the list of jobs (ascii list)
        order for the jobs (defaults to the order of job_list)

     """
     self.file=SEP.log.logging.add_dir(file)
     self.lock_stat=0
     self.status_dict={}
     self.update=[]
     if load:
       self._lock_it()
       job_list=self.status_dict.keys()
     self._lock_it(read=None)
     self.init_all(job_list)
     self._unlock_it(write=None)
     if not order: self.order=job_list
     else: self.order=order
     self.first=1


  def init_status(self,par):
    """Set the initial status for a job"""
    SEP.util.msg( "Haven't defined how to initialize a status")
    sys.exit(-1)


  def _lock_it(self,read=1):
    """Lock a file so only this process change file contents"""
    self.lock_stat=1
    if read: self.status_dict=self.read_status()
    return

  def _unlock_it(self,write=1):
    """Unlock a file so all processes can access the status file"""
    if write:self.write_status()
    self.lock_stat=0

  def init_all(self,job_list=None):
    """Initialize all status"""
    if not job_list: job_list=self.status_dict.keys()
    for job in job_list: 
      self.init_status(job)
      self.update.append(job)
    
 
  def update_status(self,par,stat):
    """Update the status of a given job"""
    SEP.util.msg( "Haven't defined how to update a status")

  def read_status(self):
      """Read the status from a file"""
      dict={}
      f=open(self.file,"r")
#      fcntl.flock(f, fcntl.LOCK_EX)
      line=f.readline().rstrip()
      while line:
        dict=self.line_to_status(dict,line)
        line=f.readline().rstrip()
#      fcntl.flock(f, fcntl.LOCK_UN)
      f.close()
      return dict

  def line_to_status(self,dict,line):
     """Convert a line to internal status representation"""
     SEP.util.msg( "Haven't defined how to convert line to internal status")
     sys.exit(-1)

  def status_to_line(key,val):
     """Convert internal status to line"""
     SEP.util.msg( "Haven't defined how to convert status to line")
     sys.exit(-1)

  def print_status(self,progress):
     """Print  the status of all keys"""
     self._lock_it()
     self.status_dict=self.read_status()
     if self.status_dict:
       for val in self.order:
          self.print_status_info(val)
     self._unlock_it()

  def add_header(self,file):
    """Add a header"""

  def write_status(self):
      """Write the status to a file"""
      if not self.first: 
        dict=self.read_status()
        self.first=None
      else: dict=self.status_dict
      
      f=open(self.file,"w")
#      fcntl.flock(f, fcntl.LOCK_EX)
      self.add_header(f)
#      for key,val in self.status_dict.items():
      for key in self.order:
        if self.status_dict.has_key(key):
          val=self.status_dict[key]
          if self.update.count(key) >0:
            f.write(self.status_to_line(key,val))
          else:
            f.write(self.status_to_line(key,dict[key]))
#      fcntl.flock(f, fcntl.LOCK_UN)
      self.update=[]
      f.close()

  def print_status_info(self,stat):
     """Print status info """
     SEP.util.msg( "Print status_info undefined")
     sys.exit(-1)

  def return_keys(self):
     """Return all status keys"""
     return self.status_dict.keys()

