import SEP.sepfile
import SEP.util
import SEP.spawn
import types
import string
import SEP.paths
import SEP.pj_base
import SEP.prog
import SEP.paths
import SEP.pf_split
import SEP.pf_copy
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2004.4.11"


def window_docs():
  par_list=SEP.args.basic(name="NONE")
  par_list.add_doc_param("intag",doc="Input file")
  par_list.add_doc_param("outtag",doc="Output file",required=None)
  par_list.add_doc_param("verb",doc="Whether or not print  ", required=None)
  par_list.add_doc_param("squeeze",doc="Whether or not to squeeze axes of length 1", required=None)
  par_list.add_doc_param("n1",doc="Number of samples in the first output axis",required=None)
  par_list.add_doc_param("f1",doc="First sample along the first axis",required=None)
  par_list.add_doc_param("j1",doc="Subsampling for the first axis",required=None)
  par_list.add_doc_param("max1",doc="Maximum value along the first axis ",required=None)
  par_list.add_doc_param("min1",doc="Minumum value along the first axis ",required=None)
  par_list.add_doc_param("n2",doc="Number of samples in the second output axis",required=None)
  par_list.add_doc_param("f2",doc="First sample along the second axis",required=None)
  par_list.add_doc_param("j2",doc="Subsampling for the second axis",required=None)
  par_list.add_doc_param("max2",doc="Maximum value along the second axis ",required=None)
  par_list.add_doc_param("min2",doc="Minumum value along the second axis ",required=None)
  par_list.add_doc_param("n3",doc="Number of samples in the third output axis",required=None)
  par_list.add_doc_param("f3",doc="First sample along the third axis",required=None)
  par_list.add_doc_param("j3",doc="Subsampling for the third axis",required=None)
  par_list.add_doc_param("max3",doc="Maximum value along the third axis ",required=None)
  par_list.add_doc_param("min3",doc="Minumum value along the third axis ",required=None)
  par_list.add_doc_param("n4",doc="Number of samples in the fourth output axis",required=None)
  par_list.add_doc_param("f4",doc="First sample along the fourth axis",required=None)
  par_list.add_doc_param("j4",doc="Subsampling for the fourth axis",required=None)
  par_list.add_doc_param("max4",doc="Maximum value along the fourth axis ",required=None)
  par_list.add_doc_param("min4",doc="Minumum value along the fourth axis ",required=None)
  par_list.add_doc_param("n5",doc="Number of samples in the fifth output axis",required=None)
  par_list.add_doc_param("f5",doc="First sample along the fifth axis",required=None)
  par_list.add_doc_param("j5",doc="Subsampling for the fifth axis",required=None)
  par_list.add_doc_param("max5",doc="Maximum value along the fifth axis ",required=None)
  par_list.add_doc_param("min5",doc="Minumum value along the fifth axis ",required=None)
  par_list.add_doc_param("n6",doc="Number of samples in the sixth output axis",required=None)
  par_list.add_doc_param("f6",doc="First sample along the sixth axis",required=None)
  par_list.add_doc_param("j6",doc="Subsampling for the sixth axis",required=None)
  par_list.add_doc_param("max6",doc="Maximum value along the sixth axis ",required=None)
  par_list.add_doc_param("min6",doc="Minumum value along the sixth axis ",required=None)
  par_list.add_doc_param("n7",doc="Number of samples in the seventh output axis",required=None)
  par_list.add_doc_param("f7",doc="First sample along the seventh axis",required=None)
  par_list.add_doc_param("j7",doc="Subsampling for the seventh axis",required=None)
  par_list.add_doc_param("max7",doc="Maximum value along the seventh axis ",required=None)
  par_list.add_doc_param("min7",doc="Minumum value along the seventh axis ",required=None)
  par_obj={}
  return  par_list
  
  

                                                                                

class window(SEP.opt_prog.options):
  """Class to do modelling"""

  def __init__(self,name="Window3d"):
    """Initialize the Window3d program"""
    SEP.opt_prog.options.__init__(self,name)
    self.add_doc_params()
    self.set_program(%s/%s"%(SEP.paths.sepbindir,"Window3d"))

  def add_doc_params(self):
    """Parameters for window"""
    self.add_doc_params(window_docs())

class  window_par(SEP.pj_base.par_job):
  def __init__(self,name="Window3d"):
    """Initialize the Window3d parallel job"""
    self.prog_pars=[]
    SEP.pj_base.par_job.__init__(self,name)

  def add_prog_param(self,par,default=None,doc=None,required=1):
    self.add_doc_param(par,default,doc,required)
    self.prog_pars.append(par)
    


  def add_doc_params(self):
    """Add doc parameters"""
    SEP.pj_base.par_job.add_doc_params(self)
    par=window_docs() 
    self.delete_object_par_params()
    self.add_pars(par)
    self.prog_pars.extend(par.return_pars())
    self.add_doc_param("restart",doc="Whether or not to restart",required=None)
    self.add_doc_param("split_axis",doc="Axis to split the window along")
    self.add_doc_param("nblock",doc="The number blocks to break the data into")

  def prep_run(self,restart=None):  
    """Check to make sure the parameters make sense"""

    ins=SEP.sepfile.sep_file(self.param("intag"))
    if ins.headers: SEP.util.err("Only working with regular data for now")

    dff_axis=int(self.param("split_axis"))
    if dff_axis < 1 or dff_axis > ins.ndims():
      SEP.util.err("split_axis must be between 1 and ndims")

    
    nblock=int(self.param("nblock"))
    sect_pars=SEP.pj_func.create_simple_parameter_list(nblock)

    par_files={}
    par_files["stdout"]=SEP.pf_split.parfile(name=self.param("outtag"),
       dff_axis=dff_axis,tag=">",usage="OUTPUT",njobs=nblock,
       restart=restart,nblock=nblock)
    par_files["stdin"]=SEP.pf_split.parfile(name=self.param("intag"),
       dff_axis=dff_axis,tag="<",usage="INPUT",njobs=nblock,
       restart=restart,nblock=nblock)

    self.add_param("files",par_files)
    order=[]
    self.add_param("sect_pars",sect_pars)
    self.add_param("program","%s/Window3d"%SEP.paths.sepbindir)
    self.add_param("global_pars",self.return_basic_params(self.prog_pars))
    SEP.pj_base.par_job.prep_run(self,restart)

