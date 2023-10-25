import SEP.args
import SEP.sepfile
import SEP.op_oc_par
import SEP.pf_copy
import SEP.pv_copy
import SEP.pv_split
import SEP.wei
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.3.14"
                                                                                
class base(SEP.wei.base):
  """A class for doing wave equation imaging"""
  def __init__(self,name="ANGWEI"):
    SEP.wei.base.__init__(self,name)
    arg=self.add_wei_params()
    self.add_pars(arg)
    self.wei_pars.add_pars(arg)

  def add_wei_params(self):
    """Add parameters needed for WEI"""
    arg=SEP.wei.base.add_wei_params(self)
    arg.add_doc_param("pcigeps",.0001,"Epsilon for ray parameter transform")
    arg.add_doc_param("apx_o",0.,"First ph in x")
    arg.add_doc_param("apx_o",0.,"First ph in x")
    arg.add_doc_param("apx_d",.0075,"First ph in x")
    arg.add_doc_param("apx_o",0.,"First ph in y")
    arg.add_doc_param("apx_n",doc="Number of  ph in x",required=None)
    arg.add_doc_param("ray_par","y","Do ph gathers")
    arg.add_doc_param("apy_n",doc="Number of  ph in y",required=None)
    arg.add_doc_param("apx_d",.0075,"Sampling ph in x")
    return arg
                                                                                

