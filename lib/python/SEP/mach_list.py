import time,re,os,commands,string,pwd
import SEP.spawn
import SEP.util
import SEP.datapath
import SEP.stat_sep
import SEP.mach_base
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.3.18"
                                                                                

class machine(SEP.mach_base.machine):
  """class containing list of potential machines to run on"""
                                                                                
  def __init__(self,mlist,nmax=2000):
    """Initialize the machine database

       mlist - the list of machines to run on
       njobs - the maximum number of jobs to run simutaneously

    """
   
    self.mlist=mlist
    SEP.mach_base.machine.__init__(self,nmax)

  def all_label_mach(self):
    """The list of machines from a list"""
    machS=re.compile('^(.+)-\d+$')    #remove commented material
    machname={}
    for mlabel in self.mlist:
      srch=machS.search(mlabel)
      if srch:  machname[mlabel]=srch.group(1)
      else: SEP.util.err("Internal error, can't interpret %s"%mlabel)
    return machname

