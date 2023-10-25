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
  par_list.add_doc_param("rect1",0,doc=" Smoothing in dimension1",required=None)
  par_list.add_doc_param("rect2",0,doc=" Smoothing in dimension2",required=None)
  par_list.add_doc_param("rect3",0,doc=" Smoothing in dimension3",required=None)
  par_list.add_doc_param("n4",doc="What to compute for 3-D volume 0-inline, 1-crossline, 2-both",required=None)
  par_list.add_doc_param("niter",5,"Number of non-linear iterations")
  par_list.add_doc_param("liter",20,"Number of linear iterations")
  par_list.add_doc_param("max_size",100,doc="Maximum memory in megabytes",required=None)
  par_list.add_doc_param("w1",doc="Patch window size dimension 1",required=None)
  par_list.add_doc_param("w2",doc="Patch window size dimension 2",required=None)
  par_list.add_doc_param("w3",doc="Patch window size dimension 3",required=None)
  par_list.add_doc_param("p1",doc="Number of patches dimension 1",required=None)
  par_list.add_doc_param("p2",doc="Number of patches dimension 2",required=None)
  par_list.add_doc_param("p3",doc="Number of patches dimension 3",required=None)
  par_list.add_doc_param("order",1,"Order of accuracy")
  par_list.add_doc_param("verb","0","Verbosity")
  par_list.add_doc_param("pmin",doc="Minimum inline dip",required=None)
  par_list.add_doc_param("pmax",doc="Maximum inline dip",required=None)
  par_list.add_doc_param("qmin",doc="Minimum crossline dip",required=None)
  par_list.add_doc_param("qmax",doc="Maximum crossline dip",required=None)
  return par_list

class sdip(SEP.opt_prog.options):
  """Class to do dip"""

  def __init__(self,name="dip"):
    """Initialize the dip program"""
    SEP.opt_prog.options.__init__(self,name)
    self.add_doc_params()
    self.set_program("%s/Sdip"%SEP.paths.sepbindir)

  def add_doc_params(self):
    """Parameters for Sdip"""
    self.add_pars(dip_docs())
    self.add_doc_param("stdin",doc="Input SEP dataset")
    self.add_doc_param("stdout",doc="Output SEP dataset")

  def prep_run(self,restart=None):  
    """Check to make sure the parameters make sense"""

    S=SEP.sepfile.sep_file(self.param("stdin"))
    if S.grid: SEP.util.err("Input should be regular SEP3D file")
    if S.headers: SEP.util.err("Input should be regular SEP3D file")
    SEP.opt_prog.options.prep_run(self,restart=None)


