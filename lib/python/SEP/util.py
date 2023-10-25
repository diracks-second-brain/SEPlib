import os,sys,time,pwd,fcntl,commands
import SEP.rc
"""A module for simple operations"""
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.3.18"
                                                                                


debug=0
file_locks={}
additional=None
testit=0



def unique():
  """Return a unique identifier for a given process"""
  return _unique_it(additional)
  

def _unique_it(a):
  """A unique descritor is return from user name, process id, and time"""
  if not a:
    additional="%s.%s.%s"%(pwd.getpwuid(os.getuid())[0],os.getpid(),str(time.time()))
  return additional
  
def fuser(test2):
  """Return the user name """
  if not test2:
    testit="%s"%(pwd.getpwuid(os.getuid())[0])
  return testit
  
def temp_desc(base="ekjfsdaj"):
    """Return a unique name based on user name and time"""
    return fuser(testit)+"."+base+"."+str(time.time())+"."+str(os.getpid())


def temp_file(base="ekjfsdaj"):
    """Return a name for a temporary file (stored in /tmp)"""
    return "/tmp/%s"%temp_desc(base)


def err(m=""):
  """Quit with an error first printing out the string m"""
  if  debug==0:
    msg( m)
    sys.exit(-1)
  else:
    raise error,m

class error(Exception):
   """A class for handling errors"""
   def __init__(self, value):
     self.value = value
   def __str__(self):
     lines=self.value.split('\n')
     msg( "\n")
     for line in lines:
       msg("     %s"%line)
     return repr() 

def msg(strng):
  """Print out a message to the screen, do a flush to guarantee immediate action"""
  lines=strng.split('\n')
  for line in lines:
    print  "     %s"%line 
  sys.stdout.flush()

def condense(list):
    """Condense a list, removing duplicate elements"""
    ltemp=list
    ltemp.sort()
    first=1 ; out=[]
    for i in ltemp:
      if first == 1:
        last=i
        first=0
        out.append(i)
      else:
        if i != last:
          out.append(i)
          last=i
    return out

def lock_file(desc):
  """Make a lock file based on a descriptor"""
  temp_file="/tmp/lockIt.%s"%str(desc)
  return temp_file

def lock_desc(desc):
  """Pause until this process is the only process with access to this descriptor"""
  file_locks[desc]=open(lock_file(desc),"w+")
  fcntl.flock(file_locks[desc].fileno(), fcntl.LOCK_EX)

def unlock_desc(desc):
  """Free up this descriptor so other processes can lock the file"""
  if not file_locks.has_key(desc):
    err("Attempt to unlock a non-locked file")
  fcntl.flock(file_locks[desc].fileno(), fcntl.LOCK_UN)
  file_locks[desc].close()
  del file_locks[desc]

def erase_list(file_list,args):
  uid=pwd.getpwuid(os.getuid())[2]
  remove=1
  if not args.has_key("verb"):  args["verb"]=0
  if not args.has_key("interactive"):  args["interactive"]=0
  if not args.has_key("force"):  args["force"]=0
  for file in file_list:
   if os.path.isfile(file):
     stata=os.stat(file)
     uid_f = os.stat(file)[4]
     if args["force"]==1: add="-f"
     elif args["interactive"]==1: add="-i"
     elif uid_f != uid ==0: add="-i"
     else: add="-f"
     if args["verb"]==1: msg("Removing %s"%file)
     if add=="-i": msg("Do you want to remove %s y/n"%file)
     st,out=commands.getstatusoutput("rm %s %s"%(add,file))
     if st!=0: msg( out)
     if  os.path.isfile(file): remove=0
   elif args["verb"]==1:  msg( "  %s doesn't exist"%file)
  return remove
                                                                                                                           
def remote_erase(args,list):
  for key,val in list.items():
    com='%s %s "%s -f %s"'%(SEP.rc.shell,key,args["program"],val)
    if args["verb"]==1:msg( "Issuing %s"%com)
    os.system(com)

