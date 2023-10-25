import SEP.sep_socket
import SEP.sepfile
import os,sys,types,commands,string,re
import SEP.util
import SEP.datapath
import SEP.paths
import SEP.pf_patch
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2004.3.11"
                                                                                
#SEP3D CLASS
class parfile(SEP.pf_patch.parfile):
  def __init__(self,**kw):
    """Initialze a SEP3d Irregular file
    
       Required parameters:


       Optional parameters:
       noverlap  - the overlap (defaults to 0)
       partition_prog - the program used to partion the data (Sep3d_split)
       collect_prog - the program used to partion the data (Distribute_join)

       SEE parent class for more parameters

    """
    SEP.pf_patch.parfile.__init__(self,**kw)
    self.type="IRREG"

    noverlap=self._option("noverlap","XXX")  
    if noverlap=="XXX": 
       noverlap=[]
       for i in self.nblock: noverlap.append(0)
    if type(noverlap)!=types.ListType: noverlap=[noverlap]
    self.noverlap=[]
    ic=0;
    for i in noverlap: 
      self.noverlap.append(str(i))
      ic+=i 
    if len(self.nblock) != len(self.noverlap):
      SEP.util.err("Length of nblock and noverlap not the same for %s"%self.file_name)
    self.partition_prog=self._option("partition_prog","%s/Sep3d_split"%SEP.paths.sepbindir)
    if self.usage != "INPUT" and ic>0:
      SEP.util.err("Can't  have an irregular file, patch it, and use if or output")
    self.collect_prog=self._option("collect_prog","%s/Distribute_join"%SEP.paths.sepbindir)

  def clone(self,**kw):
    """Clone a parallel file irregular sep3d object"""
    kout=self.kw
    for key,val in kw.items(): kout[key]=val
    if not kw.has_key("space_only") and kout.has_key("space_only"):  
      del kout["space_only"]
    a=parfile(**kout)
    return a
