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
__version = "2005.04.10"

 
def interp_docs():
  par_list=SEP.args.basic(name="AAA")
  par_list.add_doc_param("type",2,"Type of interpolation (0-nearest, 1-linear, 2-sync)")
  par_list.add_doc_param("n1out",doc="Output n1",required=None)
  par_list.add_doc_param("o1out",doc="Output o1",required=None)
  par_list.add_doc_param("d1out",doc="Output d1",required=None)

  par_list.add_doc_param("n2out",doc="Output n2",required=None)
  par_list.add_doc_param("o2out",doc="Output o2",required=None)
  par_list.add_doc_param("d2out",doc="Output d2",required=None)

  par_list.add_doc_param("n3out",doc="Output n3",required=None)
  par_list.add_doc_param("o3out",doc="Output o3",required=None)
  par_list.add_doc_param("d3out",doc="Output d3",required=None)

  par_list.add_doc_param("verb","n","Whether or not to be verbose")
  par_list.add_doc_param("maxsize",20,"Maximum memory in megabytes")
  par_list.add_doc_param("ntab",101,"Interpolation table size")
  par_list.add_doc_param("lsinc",10,"Length  of sync interpolator (must be even)")
  return par_list

class interp(SEP.opt_prog.options):
  """Class to do interp"""

  def __init__(self,name="interp"):
    """Initialize the Interp program"""
    SEP.opt_prog.options.__init__(self,name)
    self.add_doc_params()
    self.set_program("%s/%s"%(SEP.paths.sepbindir,"Interp"))

  def add_doc_params(self):
    """Parameters for Interp"""
    self.add_pars(interp_docs())
    self.add_doc_param("stdin",doc="Input SEP dataset")
    self.add_doc_param("stdout",doc="Output SEP dataset")

  def prep_run(self,restart=None):  
    """Check to make sure the parameters make sense"""

    S=SEP.sepfile.sep_file(self.param("stdin"))
    if S.grid: SEP.util.err("Input should be regular SEP3D file")
    if S.headers: SEP.util.err("Input should be regular SEP3D file")
    SEP.opt_prog.options.prep_run(self,restart=None)



