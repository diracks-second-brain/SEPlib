import re,os,string,commands

class logging:
  """Base class for loggging a message """

  def __init__(self,logdir="."):
    self.logdir=logdir
    self.count={}
    self.first=1
    self.prefix=""
    self.suffix=""
    self.no_log=None

  def set_no_log(self,nolog):
    """Set whether or not log"""
    self.no_log=nolog

  def set_prefix(self,pfx):
    """Return prefix"""
    self.prefix=pfx

  def check_first(self):
    """Actions to perform if first call"""
    if self.first: 
#      commands.getstatusoutput("rm -rf %s"%self.logdir)
      commands.getstatusoutput("mkdir %s"%self.logdir)
      self.first=None

  def add_dir(self,name):
    """Add the log directory to a file"""
    self.check_first()
    return "%s%s%s"%(self.logdir,os.sep,name)
    
  def return_prefix(self):
    """Return the prefix"""
    return self.prefix

  def set_suffix(self,sfx):
    """Return suffix"""
    self.suffix=sfx
    
  def return_suffix(self):
    """Return the suffix"""
    return self.suffix

  def valid(self,old_name):
    """Return a name with out spaces or /"""
    return string.replace(string.replace(self.reform(old_name),' ','_'),"/",".")

  def reform(self,tag):
    """Return the name with suffix and prefix added"""
    return "%s%s%s"%(self.prefix,tag,self.suffix)

  def return_log_dir(self):
    """Return log directory"""
    return self.logdir

  def new_file_name(self,tag):
    """Return the new logfile name"""
    self.check_first()
    if self.no_log: return "/dev/null"
    tag=self.valid(tag)
    if not self.count.has_key(tag): self.count[tag]=-1
    self.count[tag]+=1
    return "%s%s%s.%d"%(self.logdir,os.sep,tag, self.count[tag])
   
  def old_file_name(self,tag):
    """Return the old logfile name"""
    if self.no_log: return "/dev/null"
    tag=self.valid(tag)
    return "%s%s%s.%d"%(self.logdir,os.sep,tag,self.count[tag])

  def return_last_log(self,tag,num=50):
    """Return the contents of the last logfile"""
    if self.no_log: return "No logging"
    stat,out=commands.getstatusoutput("tail -%d %s"%(num,self.old_file_name(tag)))
    return out
