import SEP.pj_base
import SEP.util
import SEP.pf_copy
import SEP.pf_split
import types
"""
                                                                                
  Module for SEP parallel job
                                                                                
                                                                                
                                                                                
"""
                                                                                
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.3.21"

class par_job(SEP.pj_base.par_job):
  """Class to do simple parallel jobs"""
                                                                                
  def __init__(self,name="Parallel"):
    """Initialize the parallel job program"""
    SEP.pj_base.par_job.__init__(self,name)
                                                                                
  def add_doc_params(self):
    """Add the parameters for the parallel program"""
    SEP.pj_base.par_job.add_doc_params(self)
    self.delete_object_par_params()
    self.add_doc_param("command",doc="Name of the program to run")
    self.add_doc_param("tags",doc="Tags for the files")
    self.add_doc_param("files",doc="File names")
    self.add_doc_param("usage",doc="Usage for each file (INPUT,OUTPUT)")
    self.add_doc_param("nblock",doc="number of parts to break the files into")
    self.add_doc_param("axis",doc="Axis to break each file along")
    self.add_doc_param("file_type",doc="Distribution method for each file (DISTRIBUTE,COPY)",required=None)
    self.add_doc_param("restart",None,doc="Whether or not we are restarting",required=None)

  def build_check_params(self):
    """Check to make sure we have all necessary parameters to run 
        the parallel job"""
    tago=self.param("tags")
    tags=[]
    for tag in tago:
      if tag == "stdin": tags.append("<")
      elif tag == "stdout": tags.append(">")
      else: tags.append("%s="%tag)
    self.add_param("tags",tags)
    files = self.param("files")
    usage = self.param("usage")
    axis = self.param("axis")
    nblock=self.param("nblock")
    if len(tags) != len(files) or len(files)!=len(usage) or len(tags)!=len(axis):
      SEP.util.msg("n(tags)=%d n(files)=%d n(usage)=%d n(axis)=%d"%(len(tags),
        len(files),len(usage),len(axis)))
      SEP.util.err("Expecting tags,files, and usage to be the same length")
    type2 = self.param("file_type",error=None)
    if  type2:
      if len(tags) != len(type2):
        SEP.util.err("expecting tags and type to be the same length")
    else:
      type2=[]
      for i in range(len(tags)): type2.append("DISTRIBUTE")
      self.add_param("file_type",type2)
    self.add_param("program",self.param("command"))
    SEP.pj_base.par_job.build_check_params(self)
  def prep_run(self,restart=None):
    """Build the parallel files and prepare the job"""
    par_files={}
    tags = self.param("tags")
    files = self.param("files")
    usage = self.param("usage")
    axis = self.param("axis")
    file_type = self.param("file_type")
    nblock=self.param("nblock")
    if type(nblock)!=types.ListType: nblock=[nblock]
    nsect=1
    for i in nblock: nsect=nsect*int(i)
    sect_pars={}

    #create the default 
    order=[]
    for i in range(int(nsect)):
      sect_pars[str(i)]=SEP.args.basic(name=str(i))
      order.append(str(i))

    #Create the parallel files
    restart=self.param("restart")
    for i in range(len(tags)):
      if file_type[i]=="DISTRIBUTE":
        par_files[tags[i]]=SEP.pf_split.parfile(name=files[i],
         dff_axis=int(axis[i]),
          tag=tags[i],usage=usage[i],njobs=nsect,
           restart=restart,nblock=nblock)
      elif file_type[i]=="COPY":
        par_files[tags[i]]=SEP.pf_copy.parfile(name=files[i],
       tag=tags[i],usage=usage[i],njobs=nsect,restart=restart)
      else: SEP.util.err("Unknown type %s"%file_type[i])
                                                                                
    self.add_param("files",par_files)
    self.add_param("order",order)
    self.add_param("sect_pars",sect_pars)
    SEP.pj_base.par_job.prep_run(self,restart)
