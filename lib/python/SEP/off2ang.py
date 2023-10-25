import SEP.sepfile
import SEP.util
import SEP.spawn
import types
import string
import SEP.paths
import SEP.opt_prog
import SEP.parameter
import SEP.pj_base
import SEP.prog
import SEP.paths
import SEP.pf_split
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.04.10"

 
def off2ang_docs():
  par_list=SEP.args.basic(name="AA")
  par_list.add_doc_param("inverse",default="n",doc="Whether to forward or inverse tranform")
  par_list.add_doc_param("verb",doc="Whether or not to be verbose",required=None)
  par_list.add_doc_param("maxmem",200,"Maximum amount of memory to use")
  par_list.add_doc_param("eps",1.,"Epsilon parameter used by the radial trace transform")
  par_list.add_doc_param("push","n","Whether or not to push in the transform")
  par_list.add_doc_param("amax",60.,doc="Maximum angle",required=None)
  par_list.add_doc_param("nzpad",doc="Padding on the depth axis",required=None)
  return par_list

class off2ang(SEP.opt_prog.options):
  """Class to do off2ang"""

  def __init__(self,name="off2ang"):
    """Initialize the off2ang program"""
    SEP.opt_prog.options.__init__(self,name)
    self.add_doc_params()
    self.set_program("%s/%s"%(SEP.paths.sepbindir,"OFF2ANG"))

  def add_doc_params(self):
    """Parameters for OFF2ANG"""
    self.add_pars(off2ang_docs())
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


class off2ang_par(SEP.pj_base.par_job):
  """Class to do off2ang"""

  def __init__(self,name="off2ang"):
    """Initialize the off2ang program"""
    SEP.pj_base.par_job.__init__(self,name)
    self.add_doc_params()

 
  def add_doc_params(self):
    """Parameters for Stack3d"""
    self.add_pars(off2ang_docs())
    self.add_doc_param("nblock",doc="The number of blocks to break the data into ")
    self.add_doc_param("restart",doc="Whether or not we are restarting ",required=None)
    self.add_doc_param("intag",doc="Input SEP dataset (cmpx,cmpy,offset,offsety,z)[reversed for inv]")
    self.add_doc_param("outtag",doc="Output, SEP regular (z,angx,angy,cmpx,cmpy) [reversed for inv]")
    self.add_doc_param("dff_axis",doc="Axis to distribute the data along")
    self.prog_pars=["inverse","verb","maxmem","eps","push","nzpad"]
                                                                                                     

  def prep_run(self,restart=None):  
    """Check to make sure the parameters make sense"""

    restart=self.param("restart")
    S=SEP.sepfile.sep_file(self.param("intag"))
    if S.grid: SEP.util.err("Input should be regular SEP3D file")
    if S.headers: SEP.util.err("Input should be regular SEP3D file")

    self.inverse= SEP.args.logic(self.param("inverse"))
    
    #get the section parameters
    sect_pars,dff_in,dff_out=self.build_sect_params(S)

    par_files={}

    par_files["intag"]=SEP.pf_split.parfile(name=self.param("intag"), dff_axis=dff_in,
     tag="<",usage="INPUT",njobs=len(sect_pars.keys()),restart=restart)

    par_files["outtag"]=SEP.pf_split.parfile(name=self.param("outtag"),dff_axis=dff_out,
     tag=">",usage="OUTPUT",njobs=len(sect_pars.keys()),restart=restart)

    self.add_param("files",par_files)
    order=[]
    for i in range(len(sect_pars.keys())): order.append(str(i))
    self.add_param("order",order)
    self.add_param("sect_pars",sect_pars)
    axes=self.param("axes")
    self.add_param("global_pars",self.return_basic_params(self.prog_pars))
    SEP.pj_base.par_job.prep_run(self,restart)


  def build_sect_params(self,sfile):
    """Build parameters for parallel job"""
    sect_pars={}
    if self.inverse:
      if 1== int(sfile.axis(4)[0]) : 
        dff_in=4
        dff_out=2
      else: 
        dff_in=3
        dff_out=1
    else:
      if 1== int(sfile.axis(2)[0]): 
        dff_in=2
        dff_out=4
      else: 
        dff_in=1
        dff_out=3
    nblock=int(self.param("nblock"))
    for i in range(nblock):
      sect_pars[str(i)]=SEP.args.basic(name=str(i))
    return sect_pars,dff_in,dff_out


  def command_func(self,key,mlabel,par_args,job_args,tags):
    """Return the command to run given the job descriptor, job_string and file tags
                                                                                                     
       key      - the jobid
       mlabel   - the machine label we will be running on
       par_args - the parameters for the parallel jobs
       job_args - the parameters for job args
       tags     - the local file tags
                                                                                                     
       This should be overwritten when doing more complex jobs
                                                                                                     
    """
    off2ang="%s/OFF2ANG"%SEP.paths.sepbindir
    transp ="%s/Transp"%SEP.paths.sepbindir
    off_l=self.prog_list.prepare(off2ang,self.mach.mach_from_label(mlabel))
    tra_l=self.prog_list.prepare(transp ,self.mach.mach_from_label(mlabel))
    maxsize=self.param("maxmem")
    
    if tags[0][0]=="<": 
      input =tags[0]
      output=tags[1]
    else:
      input =tags[1]
      output=tags[0]

    if self.inverse:
      run="%s  %s sep.begin_par=1  %s %s| "%(off_l,input,
        string.join(par_args.return_sep_par_list()),
        string.join(job_args.return_sep_par_list()))
      run="%s %s  %s sep.end_par=1  maxsize=%s plane=13 reshape=2,4,5  %s"%(run,tra_l,output,maxsize,
        string.join(par_args.return_sep_par_list()))
    else:
      run="%s  %s sep.begin_par=1  maxsize=%s plane=13 reshape=2,4,5 %s | "%(tra_l,input,maxsize,
        string.join(par_args.return_sep_par_list()))
      run="%s %s  %s sep.end_par=1  %s %s "%(run,off_l,output,
        string.join(par_args.return_sep_par_list()),
        string.join(job_args.return_sep_par_list()))

    return run


