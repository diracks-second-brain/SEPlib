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

 
def dip_docs():
  par_list=SEP.args.basic(name="AAA")
  par_list.add_doc_param("nwind",doc="Moving window caculate dip (puck_s case) defaults to min(5,n) integer of length ndim",required=None)
  par_list.add_doc_param("verb",doc="Whether or not to be verbose",required=None)
  par_list.add_doc_param("niter",30,"Number of linear iterations")
  par_list.add_doc_param("nl_iter",10,"Number of non-linear iterations")
  par_list.add_doc_param("eps",doc="Amount of smoothing (defaults to .01[sergery_s] .0001 [sergey_f]",required=None)
  par_list.add_doc_param("eps2",.0001,doc="Amount of smoothing along axis 2 and 3 [sergey_f]")
  par_list.add_doc_param("method","sergey_f","Method (sergey_s, sergey_f, puck_f, puck_s")
  par_list.add_doc_param("v3d","n","Whether or not to run 3-D version (more memory intensive and slower)")
  return par_list

class dip(SEP.opt_prog.options):
  """Class to do dip"""

  def __init__(self,name="dip"):
    """Initialize the dip program"""
    SEP.opt_prog.options.__init__(self,name)
    self.add_doc_params()
    self.set_program("%s/Dip"%SEP.paths.sepbindir)

  def add_doc_params(self):
    """Parameters for OFF2ANG"""
    self.add_pars(dip_docs())
    self.add_doc_param("stdin",doc="Input SEP dataset")
    self.add_doc_param("stdout",doc="Output SEP dataset")

  def prep_run(self,restart=None):  
    """Check to make sure the parameters make sense"""

    S=SEP.sepfile.sep_file(self.param("stdin"))
    if S.grid: SEP.util.err("Input should be regular SEP3D file")
    if S.headers: SEP.util.err("Input should be regular SEP3D file")
    SEP.opt_prog.options.prep_run(self,restart=None)


