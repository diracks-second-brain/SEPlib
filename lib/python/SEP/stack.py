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
import SEP.pf_irreg
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.04.11"


class  stack_par(SEP.pj_base.par_job):
  def __init__(self,name="Stack3d"):
    """Initialize the stacking program"""
    SEP.pj_base.par_job.__init__(self,name)
    self.add_doc_params()
 
  def add_doc_params(self):
    """Parameters for Stack3d"""
    SEP.pj_base.par_job.add_doc_params(self)
    self.add_doc_param("axes",doc="Axes through which to compress",required=None)
    self.add_doc_param("verb",doc="Whether or not to be verbose",required=None)
    self.add_doc_param("normalize","y","Whether (y) or not (n) to normalize by the number of traces in each bin")
    self.add_doc_param("keep_headers","n","Whether or not keep the headers")
    self.add_doc_param("maxsize",200,"Maximum amount of memory to use")
    self.add_doc_param("intag",doc="Input, SEP3D, irregular volume  ")
    self.add_doc_param("outtag",doc="Output, SEP regular cube, stacked volume")
    self.add_doc_param("dff_axis",doc="Axis to distribute the data along")
    self.add_doc_param("nblock",doc="The number of blocks to break the data into ")
    self.add_doc_param("restart",doc="Whether or not we are restarting ",required=None)
    self.prog_pars=["axes","verb","normalize","keep_headers","maxsize"]

  def prep_run(self,restart=None):  
    """Check to make sure the parameters make sense"""

    restart=self.param("restart")
    S=SEP.sepfile.sep_file(self.param("intag"))
    if not S.grid: SEP.util.err("Input should be irregular SEP3D file")

    #get the section parameters
    sect_pars,dff_axes=self.build_sect_params(S)

    par_files={}
    par_files["intag"]=SEP.pf_irreg.parfile(name=self.param("intag"), 
     dff_axis=dff_axes, tag="<",usage="INPUT",njobs=len(sect_pars.values()),
      restart=restart,nblock=self.param("nblock"))

    par_files["outtag"]=SEP.pf_split.parfile(name=self.param("outtag"),
     dff_axis=dff_axes, tag=">",usage="OUTPUT",
      njobs=len(sect_pars.values()),restart=restart,nblock=self.param("nblock"))

    self.add_param("files",par_files)
    order=[]
    for i in range(len(sect_pars.keys())): order.append(str(i))
    self.add_param("order",order)
    self.add_param("sect_pars",sect_pars)
    axes=self.param("axes")
    if not axes:
      self.add_param("program","%s/Infill3d"%SEP.paths.sepbindir)
    else:
      self.add_param("program","%s/Stack3d"%SEP.paths.sepbindir)
    self.add_param("global_pars",self.return_basic_params(self.prog_pars))
    SEP.pj_base.par_job.prep_run(self,restart)

  def build_sect_params(self,sfile):
    """Build parameters for parallel job"""
    sect_pars={}
    axes=self.param("axes")
    if not axes: axes=1
    axes=int(axes)
    if axes <1 or axes >= sfile.ndims():
      SEP.util.err("Illegal axis parameter must be >=1  and < %s"%str(sfile.ndims))
    dff_axis=[]
    dff_axis.append(int(self.param("dff_axis")))
    if dff_axis[0]  <2 or dff_axis[0] > sfile.ndims:
      SEP.util.err("Must distribute along an axis that isn't compressed and not 1")

    nblock=int(self.param("nblock"))
    for i in range(nblock):
      sect_pars[str(i)]=SEP.args.basic(name=str(i))
    return sect_pars,dff_axis

