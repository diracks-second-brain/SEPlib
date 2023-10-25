import SEP.sepfile
import SEP.util
import SEP.spawn
import types
import string
import SEP.paths
import SEP.parameter
import SEP.pj_base
import SEP.prog
import SEP.opt_prog
import SEP.paths
import SEP.pf_split
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.04.8"

 
def transf_docs():
  par_list=SEP.args.basic(name="AA")
  par_list.add_doc_param("verb",doc="Whether or not to be verbose",required=None)
  par_list.add_doc_param("maxsize",1000,"Maximum amount of memory to use")
  par_list.add_doc_param("wei","y","Whether or not we are using WEI standard for frequency transforms")
  par_list.add_doc_param("f_min","0.","Minimum frequency")
  par_list.add_doc_param("f_min1",2.,doc="Minimum frequency to be preserved")
  par_list.add_doc_param("f_max","60.","Maximum frequency")
  par_list.add_doc_param("f_max1",56.,doc="Maximum frequency to be preserved")
  par_list.add_doc_param("shift2",0,doc="The amount to shift the 2: axes, useful for 2-D transforms",required=None)
  par_list.add_doc_param("inverse","y",doc="Whether or not we are running the adjoint or inverse transform when going back to time",required=None)
  return par_list

class transf(SEP.opt_prog.options):
  """Class to do transf"""

  def __init__(self,name="transf"):
    """Initialize the transf program"""
    SEP.opt_prog.options.__init__(self,name)
    self.add_doc_params()
    self.set_program("%s/%s"%(SEP.paths.sepbindir,"Transf"))

  def add_doc_params(self):
    """Parameters for Transf"""
    self.add_pars(transf_docs())
    self.add_doc_param("stdin",doc="Input SEP dataset (cmpx,cmpy,offset,offsety,z)[reversed for inv]")
    self.add_doc_param("stdout",doc="Output, SEP regular (z,angx,angy,cmpx,cmpy) [reversed for inv]")
    self.add_doc_param("verb","n","Whether or not to be verbose")

  def prep_run(self,restart=None):  
    """Check to make sure the parameters make sense"""

    S=SEP.sepfile.sep_file(self.param("stdin"))
    if S.grid: SEP.util.err("Input should be regular SEP3D file")
    if S.headers: SEP.util.err("Input should be regular SEP3D file")

    self.inverse= SEP.args.logic(self.param("inverse"))
    #get the section parameters
    SEP.opt_prog.options.prep_run(self,restart)


class transf_par(SEP.pj_base.par_job):
  """Class to do transf_docs"""

  def __init__(self,name="transf_docs"):
    """Initialize the transf_docs program"""
    SEP.pj_base.par_job.__init__(self,name)
    self.add_doc_params()

 
  def add_doc_params(self):
    """Parameters for Stack3d"""
    doc_pars=transf_docs()
    self.add_pars(doc_pars)
    self.add_doc_param("nblock",doc="The number of blocks to break the data into ",required=None)
    self.add_doc_param("restart",doc="Whether or not we are restarting ",required=None)
    self.add_doc_param("intag",doc="Input SEP dataset (t,cmpx,cmpy,hx,hy)[reversed for inv]")
    self.add_doc_param("outtag",doc="Output, SEP regular (cmpx,cmpy,hx,hy,f) [reversed for inv]")
    self.prog_pars=doc_pars.return_pars()
    SEP.pj_base.par_job.add_doc_params(self)
                                                                                                     

  def prep_run(self,restart=None):  
    """Check to make sure the parameters make sense"""

    restart=self.param("restart")
    inS=SEP.sepfile.sep_file(self.param("intag"))
    esize=int(inS.history.param("esize",4))
    shift2=int(self.param("shift2",0))
    if esize==4:
      if shift2==0: test=[4,3]
      else: test=[3,2]
    else:
      if shift2==0: test=[3,2]
      else: test=[2,1]

    isplit_in=test[0]
    if inS.axis(test[0])[0]==1:  isplit_in=test[1]
    nparts=inS.axis(isplit_in)[0]

    if esize==4: isplit_out=isplit_in+shift2-1
    else: isplit_out=isplit_in-shift2+1


    nblock=int(self.param("nblock",nparts))

    #get the section parameters
    sect_pars=SEP.pj_func.simple_parameter_list(nblock)
  
    par_files={}
    par_files["intag"]=SEP.pf_split.parfile(name=self.param("intag"), 
     dff_axis=isplit_in, tag="<",usage="INPUT",njobs=len(sect_pars.keys()),
      restart=restart,nblock=nblock)

    par_files["outtag"]=SEP.pf_split.parfile(name=self.param("outtag"),
      dff_axis=isplit_out,tag=">",usage="OUTPUT",njobs=len(sect_pars.keys()),
       restart=restart,nblock=nblock)

    self.add_param("files",par_files)
    order=[]
    for i in range(len(sect_pars.keys())): order.append(str(i))
    self.add_param("order",order)
    self.add_param("sect_pars",sect_pars)
    axes=self.param("axes")
    self.add_param("program","%s/Transf"%SEP.paths.sepbindir)
    self.add_param("global_pars",self.return_basic_params(self.prog_pars))
    SEP.pj_base.par_job.prep_run(self,restart)

