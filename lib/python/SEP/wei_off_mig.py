import SEP.args
import SEP.sepfile
import SEP.op_oc_par
import SEP.pf_copy
import SEP.pv_copy
import SEP.pv_split
import SEP.wei_op 
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.4.10"
                                                                                

class migration(SEP.wei_op.operator):
  def __init__(self,name="OFF_MIG"):
    """Operator to do offset migration"""
    SEP.wei_op.operator.__init__(self,name)
  def add_doc_params(self):
    """Documenation for offset migration"""
    SEP.wei_op.operator.add_doc_params(self)
    self.add_doc_param("wave_out",doc="File name for the wavefield at bottom of migration domain",required=None)
    self.add_doc_param("zero_image",doc="Zero-offset image",required=None)

  def add_wei_params(self):
    """Add parameters needed for WEI"""
    wei_pars=SEP.wei.base.add_wei_params(self)
    wei_pars.add_doc_param("jmx_image",'1',"Output every jth gather in midpoint x")
    wei_pars.add_doc_param("jmy_image",'1',"Output every jth gather in midpoint y")
    wei_pars.add_doc_param("image_real",'y',"Whether or not the image should be real")
    return wei_pars
 

  def add_optional_files(self,parfiles,nsect):
    if self.param("zero_image"):
      parfiles["zero_image"]=SEP.pf_copy(name=self.param("zero_image"),
       tag="zero_image=", njobs=nsect)
    if self.param("wave_out"):
      parfiles["wave_out"]=SEP.pf_copy(name=self.param("wave_out"),
       tag="wave_out=", njobs=nsect, dff_axis=5, nblock=[nsect])
    return parfiles

  def run_in_flow(self):
    return 1

  def add_image_file(self,parfiles,nsect):
    """Add the image  parallel file description"""
    if not self.logic("image_real"):
      p=SEP.pv_copy.cmplx_vector(name=self.param("R"),reuse_par="add=y",
       tag="R=",njobs=nsect, space_only=1)
    else:
      p=SEP.pv_copy.float_vector(name=self.param("R"),reuse_par="add=y",
       tag="R=",njobs=nsect, space_only=1)
    parfiles["domain"]=self.add_axes(p,["amx","amy","ahx","ahy","az_"])
    return parfiles
                                                                                


  def prep_run(self,restart=None):
     """Migrate the data"""
     SEP.wei_op.operator.prep_run(self,restart)
     self.parfiles["domain"].create_data()
     self.adjoint_op(self.parfiles["domain"],self.parfiles["range"],restart)
