import SEP.spawn
import os,pwd


__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.3.22"
                                                                                


class distribute:
  """A program that is distributed to the nodes as necessary"""
  def __init__(self,prog):
    """Initialize a distributed program object 

        prog -  The program to distribute

    """
    self.mach_list=[]
    self.prog_to  ="/tmp/%s.%s.%s"%(os.path.basename(prog),os.getpid(),
      pwd.getpwuid(os.getuid())[0])
    self.prog_erase  ="/tmp/%s.*.%s"%(os.path.basename(prog),
      pwd.getpwuid(os.getuid())[0])
    SEP.spawn.run_wait("rm -f %s"%self.prog_to,99, error=None,quiet=1)
    SEP.spawn.run_wait("cp %s %s "%(prog,self.prog_to),99)
                                                                                           
  def local_prog(self):
     """Return the local program name"""
     return self.prog_to

  def prepare(self,mach):
    """Make sure a program is on the specified local machine"""
    if self.mach_list.count(mach)==0:
      self.mach_list.append(mach)
      SEP.spawn.run_wait(SEP.rc.remote_com(mach,'rm -f %s'%self.prog_erase),oper_len=30,error=None,ntrys=5)
      SEP.spawn.run_wait(SEP.rc.cp_to(mach,self.prog_to,self.prog_to),160,
       ntrys=5)
 
  def clean(self):
    """Remove the program from the nodes"""
    for mach in self.mach_list:
      SEP.spawn.run_wait(SEP.rc.remote_com(mach,"rm -f %s"%self.prog_erase),
      oper_len=30,error=None,ntrys=5)
    self.mach_list=[]
    SEP.spawn.run_wait('rm -f %s'%self.prog_to)
                                                                                           

class distrib_list:
  """A class for distributing programs"""
  def __init__(self):
    self.dict={}
  def check_add_prog(self,prog):
    """Add an additional program to be distributed if it doesn't exist in the list"""
    base=os.path.basename(prog)  
    if not self.dict.has_key(base): self.dict[base]=distribute(prog)
    return self.dict[base]
  def clean(self):
    """Clean distributed program from all nodes"""
    for  prog in self.dict.values(): prog.clean()
    self.dict={}
  def prepare(self,prog,mach):
    """Make sure the distributed program is own the node, return local name"""
    base=os.path.basename(prog)  
    self.check_add_prog(prog).prepare(mach)
    return self.dict[base].local_prog()
