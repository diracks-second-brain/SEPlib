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
                                                                                
class offset_modeling(SEP.wei_op.operator):
  def __init__(self,name="ANG_MODEL"): 
     """Operator to do angle modeling"""
     SEP.wei_op.operator.__init__(self,name)
     self.del_axes_params(["mx","my","hx","hy","z_"])


  def prep_run(self,restart=None):
    """Check to make sure everything is setup before running"""
    rsep=SEP.sepfile.sep_file(file=self.param("R"))
    self.set_axis_param(rsep,1,"mx")
    self.set_axis_param(rsep,2,"my")
    self.set_axis_param(rsep,3,"hx")
    self.set_axis_param(rsep,4,"hy")
    n,o,d,label,unit=rsep.axis(5)
    self.add_param("az__n",n)
    self.add_param("az__o",o)
    self.add_param("az__d",d)
#    SEP.wei.base.prep_run(self,restart)
#    SEP.wei_op.operator.prep_run(self,restart)
    SEP.wei.base.prep_run(self,restart)
    self.parfiles["range"].create_data()
    self.forward_op(self.parfiles["domain"],self.parfiles["range"],restart)



