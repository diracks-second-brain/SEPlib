import types,string,os,re,pwd
import SEP.sepfile
import SEP.util
import SEP.datapath
import SEP.spawn
import SEP.vec_oc
import SEP.log
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.7.30"

class vector(SEP.vec_oc.vector,SEP.sepfile.sep_file):
  """ A class for out of core vectors in SEPlib file format"""

  def __init__(self,name=None,tag=None,vector=None,space_only=None,extra_args=" "):
     """Initialize a sep vector

        space_only - only initialze description don't wory about data
        extra_args - extra arguments that should be added when ever using
                      this vector (maybe needed for large files)

        Several different methods to initialize

        Initialze a vector from a file
          b=SEP.vector.sep_vector(tag) 
 
        Initialze a vector from another vector
          b=SEP.vector.sep_vector("aa",vector=a)
    
        Initialze an empty vector that we will define shape later
          b=SEP.vector.sep_vector("aa")

     """

     if not name: 
       if tag: self.sep_vec_name=tag
       else : SEP.util.err("Must initialize vector with a name")
     else: self.sep_vec_name=name
     SEP.vec_oc.vector.__init__(self,self.sep_vec_name)
     self.extra_args=extra_args
     self.e_size=None
     self.clones=0
     if tag:
       self._tag_init(tag)
     elif vector:
       self._vector_init(vector,space_only)
     else:
       SEP.sepfile.sep_file.__init__(self,name=self.vec_name())
     self.space_only=space_only

  def file_name(self):
    """Return the file name associated with the vector"""
    return SEP.sepfile.sep_file.file_name(self)

  def add_extra_args(self,args):
    """Add extra args for programs  using this vector"""
    self.extra_args="%s %s"%(self.extra_args,args)

  def clone_space(self,name=None):
    """Clone just the space of a vector give it the name 'name'"""
    return self.clone(name=name,space_only=1)
  
  def solver_func(self,args,file2=None):
    """Run a mathemattical operation using Solvver_ops
  
       args     - a list of arguments for Math

    """
    args_to=["Solver_ops","verb=y",">/dev/null"]
    if type(args) != types.ListType:args=args.split()
    args_to.extend(args)
    args_to.append("file1=%s"%self.vec_name())
    if file2:
      args_to.append("file2=%s"%file2.vec_name())
    tfile=SEP.util.temp_file("solver")
    SEP.spawn.run_wait(args_to,logfile=SEP.log.logging.new_file_name("solver_op"))
    lines=SEP.log.logging.return_last_log("solver_op").split("\n")
    return string.join(lines,":::")
  
  
  def _tag_init(self,tag):
    """Initialize a vector from a septag"""
    SEP.sepfile.sep_file.__init__(self,file=tag,name=self.vec_name())

  def _vector_init(self,vector,space_only):
    """Initialize a sepvector from another vector"""
    arg=" "
    SEP.sepfile.sep_file.__init__(self,sepfile=vector,name=self.vec_name())
    if self.headers:
      infile,err=self.header_par("in")
      self.set_header_par("in",infile)
    if self.grid:
      infile,err=self.grid_par("in")
      self.set_grid_par("in",infile)
    if not space_only: 
      if vector.space_only:self.create_data()
      else: self.copy_data(vector)

  def copy_data(self,vector):
    """Copy the data from one vector to another"""
    SEP.util.err("copy_data must be overwritten")
  def create_data(self,vector):
    """Create the data for a vector"""
    SEP.util.err("copy_data must be overwritten")

  def rand(self):
     """out=random numbers"""
     self.solver_func("op=rand")

  def zero(self):
    """Zero a vector"""
    SEP.util.err("zero must be overwritten")

  def clean(self):
     """Remove a vector"""
     if not self.space_only: SEP.spawn.run_wait(["Rm3d",self.vec_name()],verb=verbage)
