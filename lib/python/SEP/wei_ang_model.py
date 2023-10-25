import SEP.args
import SEP.sepfile
import SEP.op_oc_par
import SEP.pf_copy
import SEP.pv_copy
import SEP.pv_split
import SEP.wei_ang
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.3.14"
                                                                                

class modeling(SEP.wei_ang.base):
  def __init__(self,name="ANG_MODEL"): 
     """Operator to do angle modeling"""
     SEP.wei_ang.base.__init__(self,name)
     self.del_axes_params("mx","my","px","py","w_")
     self.del_doc_param("ahx_n")
     self.del_doc_param("ahy_n")

  def prep_run(self,restart=None):
    """Check to make sure everything is setup before running"""
    rsep=SEP.sepfile.sep_file(file=self.param("R"))
    self.set_axis_param(self,rsep,1,"mx")
    self.set_axis_param(self,rsep,2,"my")
    self.set_axis_param(self,rsep,3,"px")
    self.set_axis_param(self,rsep,4,"py")
    self.add_param("ahy_n",self.param("apy_n"))
    self.add_param("ahx_n",self.param("apx_n"))
    SEP.wei_ang.base.prep_run(self,restart)
    self.parfiles["range"].create_data()
    self.forward_op(self.parfiles["domain"],self.parfile["range"],restart)

