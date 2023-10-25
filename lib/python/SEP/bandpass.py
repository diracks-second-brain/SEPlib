import SEP.sepfile
import SEP.util
import SEP.spawn
import types
import string
import SEP.paths
import SEP.opt_prog
import SEP.parameter
import SEP.pj_base
import SEP.prog
import SEP.paths
import SEP.pf_split
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.03.22"

def bandpass_docs():
  par_list=SEP.args.basic(name="AA")
  par_list.add_param("flo",None,default=0.,doc="Low frequency cutoff")
  par_list.add_param("fhi",None,doc="High frequency cutoff",required=None)
  par_list.add_param("f3db",None,doc=" 3 DB reduction point",required=None)
  par_list.add_param("nphi",None,doc="Number of poles high cutoff",required=None)
  par_list.add_param("nplo",None,doc="Number of poles low cutoff",required=None)
  par_list.add_param("verb",None,default="n",doc="Whether or not be verbose")
  par_list.add_param("phase",None,default=0,doc="Zero phase (0) or minumum phase (1)")
  return par_list

class bandpass(SEP.opt_prog.options):
  """Class to do bandpass"""

  def __init__(self,name="interp"):
    """Initialize the Bandpass program"""
    SEP.opt_prog.options.__init__(self,name)
    self.add_doc_params()
    self.set_program("%s/%s"%(SEP.paths.sepbindir,"Bandpass"))

  def add_doc_params(self):
    """Parameters for Bandpass"""
    self.add_pars(bandpass_docs())
    self.add_doc_param("stdin",doc="Input SEP dataset")
    self.add_doc_param("stdout",doc="Output SEP dataset")

