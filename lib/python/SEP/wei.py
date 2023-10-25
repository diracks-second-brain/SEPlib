import SEP.opt_prog
import SEP.args
import SEP.sepfile
import SEP.op_oc_par
import SEP.pf_copy
import SEP.pv_copy
import SEP.pv_split
import SEP.paths
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.4.10"
                                                                                

class base(SEP.op_oc_par.operator):
  """A class for doing wave equation imaging"""
  def __init__(self,name="WEI"):
     SEP.op_oc_par.operator.__init__(self,name)
     self.name=name
     self.add_doc_params()

  def add_doc_params(self):
    """Documentation for wei operator"""
    SEP.pj_base.par_job.add_doc_params(self)
    self.wei_pars=self.add_wei_params()
    self.add_pars(self.add_distrib_params())
    self.add_pars(self.add_additional_pars())
    self.add_pars(self.wei_pars)
    self.delete_object_par_params()


  def add_additional_pars(self):
   """Add additional parameters"""
   pars=SEP.args.basic(name="additional")
   pars.add_doc_param("nsect",doc="The number of sections to break the w into",required=None)
   pars.add_doc_param("program","%s/Phase",SEP.paths.sepbindir,"Path of WEI based program")
   return pars
  
  def add_optional_files(self,parfiles,nsect):
    """Add any optional files requested by user"""
    return parfiles

  def prep_run(self,restart=None):
    """Check to see if parameters make sense right before a run"""
    self.parfiles=self.build_parfiles()
    sect,order=self.sect_pars()
    self.add_param("order",order)
    self.add_param("sect_pars",sect)
    self.add_param("adj_com","adj=y")
    self.add_param("files",self.parfiles)
    self.add_param("nw_tot",self.param("aw__n"))
    self.wei_pars.add_param("nw_tot",self.param("aw__n"))
    nzb=self.param("nzb")
    if not nzb: 
      nzb=100*1000*125/int(self.param("amx_n"))/int(self.param("amy_n"))
      nzb=nzb/int(self.param("ahx_n"))/int(self.param("ahy_n"))
      nzb=max(1,min(int(self.param("az__n")),nzb))
      self.add_param("nzb",nzb)
    
    self.add_param("global_pars",self.return_basic_params(self.wei_pars.return_pars()))
    SEP.op_oc_par.operator.prep_run(self,restart)


  def sect_pars(self):
    """Return the section paramters  for the jobs"""
    nw=int(self.param("aw__n"))
    ow=float(self.param("aw__o"))
    dw=float(self.param("aw__d"))
    nsect=int(self.param("nsect"))
    par_dict={}
    order=[]
    nsect=int(nsect)
    nb=int(int(nw)/int(nsect))
    nextra=nw-nsect*nb
    itot=0
    for i in range(nsect):
       key=str(i)
       order.append(key)
       par_dict[key]=SEP.args.basic(name=str(i))
       o=ow+dw*itot
       n=nb
       if i < nextra: n=n+1
       par_dict[key].add_param("aw__n",n)
       par_dict[key].add_param("aw__o",o)
       par_dict[key].add_param("aw__d",dw)
       itot=itot+n
    return par_dict,order

  def build_parfiles(self):
    """Build the parallel files neeeded for the run"""
    nsect=self.param("nsect")
    if not nsect: nsect=self.param("aw__n")
    self.add_param("nsect",nsect)
    parfiles=self.check_slownesses(nsect)
    parfiles=self.add_image_file(parfiles,nsect)
    parfiles=self.add_data_file(parfiles,nsect)
    return self.add_optional_files(parfiles,nsect)

  def add_image_file(self,parfiles,nsect):
    """Add the data parallel file description"""
    p=SEP.pv_copy.cmplx_vector(name=self.param("R"),reuse_par="add=y",
      tag="R=",njobs=nsect,space_only=1)
    parfiles["domain"]=self.add_axes(p,["amx","amy","ahx","ahy","az_"])
    return parfiles

  def add_data_file(self,parfiles,nsect):
    """Add the data parallel file description"""
    p=SEP.pv_split.cmplx_vector(name=self.param("D"),tag="Data=",
      njobs=nsect,space_only=1, dff_axis=5, nblock=[nsect])
    parfiles["range"]=self.add_axes(p,["amx","amy","ahx","ahy","aw_"])
    return parfiles

  def add_axes(self,vector,axes):
    """Add axes information to a file"""
    for i in range(len(axes)):
      par=axes[i]
      n=self.param("%s_n"%par)
      o=self.param("%s_o"%par)
      d=self.param("%s_d"%par)
      vector.set_axis(i+1,n,o,d,par,"none")
    return vector


  def check_slownesses(self,nsect):
    """Check to make sure the slownesses make sense, return the parallel file representations"""
    parfiles={}
    sfile=SEP.sepfile.sep_file(file=self.param("S"))
    SEP.sep_ff.axis_same("az_",self.param("az__n"),self.param("az__o"),
     self.param("az__d"),"Slowness axis 3",sfile.axis(3)[0],sfile.axis(3)[1],
      sfile.axis(3)[2])
    parfiles["S"]=SEP.pf_copy.parfile(name=self.param("S"),
      tag="S=",njobs=nsect, usage="INPUT",type="COPY")

    sfile=SEP.sepfile.sep_file(file=self.param("schoice"))
    SEP.sep_ff.axis_same("az_",self.param("az__n"),self.param("az__o"),
     self.param("az__d"),"schoice axis 3",sfile.axis(3)[0],sfile.axis(3)[1],
      sfile.axis(3)[2])
    if int(sfile.axis(2)[0]) != 2:
      SEP.util.err("Expecting schoice n2=2  (result of Lloyd_vel) got n2=%d"%sfile.axis(2)[0])
    parfiles["schoice"]=SEP.pf_copy.parfile(name=self.param("schoice"),
      tag="ref_slow=",njobs=nsect, usage="INPUT",type="COPY")
    return parfiles
    
  def add_distrib_params(self):
    arg=SEP.args.basic(name="par")
    arg.add_doc_param("program",doc="WEI program to run")
    arg.add_doc_param("R",doc="Image (mx,my,hx,hy,w)")
    arg.add_doc_param("S",doc="Slowness file (x,y,z)")
    arg.add_doc_param("D",doc="Data file (mx,my,hx,hy,w)")
    arg.add_doc_param("schoice",doc="Slowness reference (nfef,2,nz)")
    arg.add_doc_param("nsect",doc="The number of jobs to run (defaults to nfreq)",
     required=None)
    return arg

  def add_wei_params(self):
    """Add standard parameters that are used by wei"""
    pars=SEP.args.basic(name="WEI_pars")
    pars.add_doc_param("verb",doc="Verbosity level",required=None)
    pars.add_doc_param("first_dz",doc="First dz step",required=None)
    pars.add_doc_param("velocity",'y',"Velocity (y) or slowness (n)")
    pars.add_doc_param("npad_hy",'0',"Padding in hy to kill edge effects")
    pars.add_doc_param("npad_hx",'0',"Padding in hx to kill edge effects")
    pars.add_doc_param("npad_my",'0',"Padding in cmpy to kill edge effects")
    pars.add_doc_param("npad_mx",'0',"Padding in cmpx to kill edge effects")
    pars=self.add_axis_param(pars,"z_")
    pars=self.add_axis_param(pars,"mx")
    pars=self.add_axis_param(pars,"my")
    pars=self.add_axis_param(pars,"hx")
    pars=self.add_axis_param(pars,"hy")
    pars.add_doc_param("nzb",
     doc="Size of depth block (defaults to 100MB worth)",required=None)
    return pars


  def add_axis_param(self,pars,tag):
    """Add an axis parameter"""
    pars.add_doc_param("a%s_n"%tag,doc="Number of %s "%tag)
    pars.add_doc_param("a%s_o"%tag,doc="First %s "%tag)
    pars.add_doc_param("a%s_d"%tag,doc="Sampling in %s "%tag)
    return pars
  

  def del_axes_params(self,axes):
    """Delete an axis"""
    for ax in axes:
      self.del_par("a%s_n"%ax)
      self.del_par("a%s_o"%ax)
      self.del_par("a%s_d"%ax)

  def set_axis_param(self,vec,iax,tag):
     """Set parameters based on an axis"""
     n,o,d,label,unit=vec.axis(iax)
     self.add_param("a%s_n"%tag,int(n))
     self.add_param("a%s_o"%tag,float(o))
     self.add_param("a%s_d"%tag,float(d))


