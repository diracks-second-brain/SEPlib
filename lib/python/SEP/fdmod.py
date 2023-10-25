import SEP.sepfile
import SEP.util
import SEP.spawn
import types
import string
import SEP.paths
import SEP.opt_prog
import SEP.pj_base
import SEP.paths
import SEP.pf_split
import SEP.pf_copy
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2004.4.12"


def fdmod_docs():
  par_list=SEP.args.basic(name="NONE")
  par_list.add_doc_param("intag",doc="Velocity file n1=nz,n2=nx ")
  par_list.add_doc_param("outtag",doc="File containing waves  n1=nz,n2=nx,n3=nt ",required=None)
  par_list.add_doc_param("no_stdout",default=1,doc="Whether of not to write out wavefield")
  par_list.add_doc_param("tmax",doc="Maximum time to model")
  par_list.add_doc_param("nt",doc="Number of time samples (calculated based on stability)", required=None)
  par_list.add_doc_param("mt",doc="Number of time steps per output time step", required=None)
  par_list.add_doc_param("fmax",doc="Maximum frequency in source wavelet vmin/(10.*h)", required=None)
  par_list.add_doc_param("fpeak",doc="Peak frequency in source wavelet .5*fmax", required=None)
  par_list.add_doc_param("dfile",doc="Density file", required=None)
  par_list.add_doc_param("vsx",doc="X coordinate of vertical line of seismograms (o2)", required=None)
  par_list.add_doc_param("hsz",doc="Z coordinate of horizontal line of seismograms(o1) ", required=None)
  par_list.add_doc_param("bx",doc="Samples along x axis left of sources [(xs[0]-o2)/d2] ", required=None)
  par_list.add_doc_param("ex",doc="Samples along x axis right of sources [(max(axis2) xs[0])/d2] ", required=None)
  par_list.add_doc_param("bz",doc="Samples along z axis left of sources [(zs[0]-o1)/d1] ", required=None)
  par_list.add_doc_param("ez",doc="Samples along z axis right of sources [(max(axis1)-zs[0])/d1] ", required=None)
  par_list.add_doc_param("hsfile",doc="Horizontal line of seismograms ", required=None)
  par_list.add_doc_param("vsfile",doc="Vertical line of seismograms ", required=None)
  par_list.add_doc_param("verb",doc="Whether or not print  ", required=None)
  par_list.add_doc_param("print_master",default=2.,doc="Print frequency  ")
  par_list.add_doc_param("abs",default=[1,1,1,1],doc="Absoring  boundaries top,left,bottom,right  ")
  par_list.add_doc_param("pml_max",default=1000.,doc="PML absoption parameter")
  par_list.add_doc_param("jt",default=1,doc="Subsampling of time axis for hsfile and vsfile",required=None)
  par_list.add_doc_param("pml_thick",default=0,doc="half-thickness of pml layer",required=None)
  return  par_list
  
  

                                                                                

class fdmod(SEP.opt_prog):
  """Class to do modelling"""

  def __init__(self,name="Fdmod"):
    """Initialize the Fdmod program"""
    SEP.opt_prog.options.__init__(self,name)
    self.add_doc_param()
    self.set_program("%s/%s"%(SEP.paths.sepbindir,"Fdmod"))

  def add_doc_params(self):
    """Parameters for fdmod"""
    self.add_doc_params(fdmod_docs())
    self.add_doc_param("xs",doc="A list of source locations in x",required=None)
    self.add_doc_param("zs",doc="A list of source locations in z",required=None)
    self.add_doc_param("nzs",doc="Number of source locations in z",required=None)
    self.add_doc_param("ozs",doc="First source locations in z",required=None)
    self.add_doc_param("dzs",doc="Sampling of source locations in z",required=None)
    self.add_doc_param("nxs",doc="Number of source locations in x",required=None)
    self.add_doc_param("oxs",doc="First source locations in x",required=None)
    self.add_doc_param("dxs",doc="Sampling of source locations in x",required=None)


class  fdmod_par(SEP.pj_base.par_job):
  def __init__(self,name="Fdmod_par"):
    """Initialize the Fdmod parallel job"""
    self.prog_pars=[]
    SEP.pj_base.par_job.__init__(self,name)

  def add_prog_param(self,par,default=None,doc=None,required=1):
    self.add_doc_param(par,default,doc,required)
    self.prog_pars.append(par)
    


  def add_doc_params(self):
    """Add doc parameters"""
    SEP.pj_base.par_job.add_doc_params(self)
    pars=fdmod_docs() 
    self.delete_object_par_params()
    self.add_pars(pars)
    self.prog_pars.extend(pars.return_pars())
    self.add_prog_param("nblock",doc="Number of blocks to break modeling into (defaults to number of shots)",required=None)
    self.add_prog_param("nzs",1,doc="Number of source locations in z")
    self.add_prog_param("ozs",0.,doc="First source locations in z")
    self.add_prog_param("dzs",1.,doc="Sampling of source locations in z")
    self.add_prog_param("nxs",1,doc="Number of source locations in x")
    self.add_prog_param("oxs",0.,doc="First source locations in x")
    self.add_prog_param("dxs",0.,doc="Sampling of source locations in x")
    self.add_doc_param("restart",doc="Whether or not to restart",required=None)

  def prep_run(self,restart=None):  
    """Check to make sure the parameters make sense"""

    S=SEP.sepfile.sep_file(self.param("intag"))
    if S.ndims() !=2: SEP.util.err("Expecting slowness to be 2-D")

    if not self.param("hsfile",error=None) and not self.param("vsfile",error=None) and not self.param("outtag",error=None):
     SEP.util.err("Must be outputing vsfile, hsfile, or wavefield")

    #get the section parameters
    sect_pars=self.build_sect_params()

    par_files={}
    par_files["vel"]=SEP.pf_copy.parfile(name=self.param("intag"),
     tag="intag=",usage="INPUT",njobs=len(sect_pars.keys()),restart=restart)

    if self.param("dfile",error=None):
      par_files["dfile"]=SEP.pf_copy.parfile(name=self.param("dfile"),
       tag="dfile=",usage="INPUT",njobs=len(sect_pars.keys()),restart=restart)

    i= self.param("no_stdout")
    if i==0: 
      par_files["wave"]=SEP.pf_split.parfile(name=self.param("outfile"),
         dff_axis=3,tag="outfile=",usage="OUTPUT",njobs=len(sect_pars.keys()),
         restart=restart,nblock=len(sect_pars.keys()))

    if self.param("hsfile",error=None): 
      par_files["hsfile"]=SEP.pf_split.parfile(name=self.param("hsfile"),
         dff_axis=3,tag="hsfile=",usage="OUTPUT",njobs=len(sect_pars.keys()),
         restart=restart,nblock=len(sect_pars.keys()))

    if self.param("vsfile",error=None): 
      par_files["vsfile"]=SEP.pf_split.parfile(name=self.param("vsfile"),
         dff_axis=3,tag="vsfile",usage="OUTPUT",njobs=len(sect_pars.keys()),
         restart=restart,nblock=len(sect_pars.keys()))

    self.add_param("files",par_files)
    order=[]
    for i in range(len(sect_pars.keys())): order.append(str(i))
    self.add_param("order",order)
    self.add_param("sect_pars",sect_pars)
    self.add_param("program","%s/Fdmod"%SEP.paths.sepbindir)
    self.add_param("global_pars",self.return_basic_params(self.prog_pars))
    SEP.pj_base.par_job.prep_run(self,restart)

    
        
  def build_sect_params(self):
    """Build parameters for parallel job"""
    sect_pars={}
    nxs=int(self.param("nxs"))
    oxs=float(self.param("oxs") )
    dxs=float(self.param("dxs")  )  
    nzs=int(self.param("nzs"))
    ozs=float(self.param("ozs")   ) 
    dzs=float(self.param("dzs") )
    if nxs !=1 and nzs !=1: 
      SEP.util.err("Can only deal with nxs or nzs not equal to 1")
    if nxs!=1: 
      n=nxs
      o=oxs
      d=dxs
    else:
      n=nzs
      o=ozs
      d=dzs
    nblock=self.param("nblock")
    if not nblock: nblock=n
    nblock=int(n)
    imin=int(n/nblock)
    nextra=n-nblock*imin
    itot=0
    for i in range(nblock):
      sect_pars[str(i)]=SEP.args.basic(name=str(i))
      ol=o+d*itot
      dl=d
      nl=imin
      if i < nextra: nl=nl+1
      itot=itot+nl
      if n==nxs:
        sect_pars[str(i)].add_string("nzs=1 ozs=%f dzs=%f"%(ozs,dzs))
        sect_pars[str(i)].add_string("nxs=%d oxs=%f dxs=%f"%(nl,ol,dl))
      else:
        sect_pars[str(i)].add_string("nxs=1 oxs=%f dxs=%f"%(oxs,dxs))
        sect_pars[str(i)].add_string("nzs=%d ozs=%f dzs=%f"%(nl,ol,dl))
    return sect_pars

    
    
