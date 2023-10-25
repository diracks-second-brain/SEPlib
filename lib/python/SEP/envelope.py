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
__version = "2005.03.21"

def envelope_docs():
  par_list=SEP.args.basic(name="AAA")
  par_list.add_doc_param("tc1",0,doc="Half-width of smoother along axis 1")
  par_list.add_doc_param("tc2",0,doc="Half-width of smoother along axis 2")
  par_list.add_doc_param("freq",1,doc="Produces instanteous frequency instead of envelopes")
  return par_list

class envelope(SEP.opt_prog.options):
  """Class to do envelope"""

  def __init__(self,name="interp"):
    """Initialize the Envelope program"""
    SEP.opt_prog.options.__init__(self,name)
    self.add_doc_params()
    self.set_program("%s/%s"%(SEP.paths.sepbindir,"Envelope"))
  def add_doc_params(self):
    """Parameters for Envelope"""
    self.add_pars(envelope_docs())
    self.add_doc_param("stdin",doc="Input SEP dataset")
    self.add_doc_param("verb","n",doc="Whether or not to be verbose")
    self.add_doc_param("stdout",doc="Output SEP dataset")


