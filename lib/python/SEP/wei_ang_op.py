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
                                                                                
class operator(SEP.wei_ang.base):
  def __init__(self,name="ANG_OP"): 
     """Operator to do angle inversion"""
     SEP.wei_ang.base.__init__(self,name)
     self.del_axes_params(["mx","my","hx","hy","w_"])
     self.del_doc_param("apx_n")
     self.del_doc_param("apy_n")

  def prep_run(self,restart):
    """Check to make sure everything is setup before running"""
    dsep=SEP.sepfile.sep_file(file=self.param("D"))
    self.set_axis_param(dsep,1,"mx")
    self.set_axis_param(dsep,2,"my")
    self.set_axis_param(dsep,3,"hx")
    self.set_axis_param(dsep,4,"hy")
    self.set_axis_param(dsep,5,"w_")
    self.add_param("apy_n",self.param("ahy_n"))
    self.add_param("apx_n",self.param("ahx_n"))
    self.add_param("ray_par","y")
    SEP.wei_ang.base.prep_run(self,restart)

