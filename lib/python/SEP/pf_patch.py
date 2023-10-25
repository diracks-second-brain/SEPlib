import SEP.sep_socket
import SEP.sepfile
import os,sys,types,commands,string,re
import SEP.util
import SEP.datapath
import SEP.paths
import SEP.pf_split
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.3.15"
                                                                                
#PATCHING CLASS
class parfile(SEP.pf_split.parfile):
  def __init__(self,**kw):
    """Initialze a patched file
    
       Required parameters:

       noverlap  - the overlap 

       Optional parameters:
       partition_prog - the program used to partion the data (Patch_split)
       collect_prog - the program used to partion the data (Patch_join)

       SEE parent class for more parameters

    """
    SEP.pf_split.parfile.__init__(self,**kw)
    self.type="PATCH"

    self.set_noverlap(self._option("noverlap",None,err=None))
    self.partition_prog=self._option("partition_prog","%s/Patch_split"%SEP.paths.sepbindir)
    self.collect_prog=self._option("collect_prog","%s/Patch_join"%SEP.paths.sepbindir)

  def clone(self,**kw):
    """Clone a parallel file patch object"""
    kout=self.kw
    for key,val in kw.items(): kout[key]=val
    if not kw.has_key("space_only") and kout.has_key("space_only"):  
      del kout["space_only"]
    a=parfile(**kout)
    return a


