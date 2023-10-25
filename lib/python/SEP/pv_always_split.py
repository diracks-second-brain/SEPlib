import string,types,re,sys
import SEP.pj_split
import SEP.util
import SEP.vec_scmplx
import SEP.vec_sfloat
import SEP.pf_split
import SEP.pv_func
import SEP.paths
import SEP.pc
import SEP.pc_mpich
import SEP.rc
import SEP.spawn
import SEP.opt_data
import SEP.log

__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2004.12.1"

class float_vector(SEP.pf_split.parfile,SEP.vec_sfloat.vector):
  def __init__(self,**kw):
     """Initialize a complex parallel vector

        Requred:
           name  - The name of the dataset 

        Optional parameters:
          exists - Whether or not the datast exists (assumed no [None])

          vector - Another vector

          sect_map - Map relating key to the machine it should be on

          space_only - (defaults to None) means we are only describing the 
           vector space (not necessarily any data associated with it)

     """
     if not kw.has_key("name"): SEP.util.err("Must provide the name of the dataset")
     name=kw["name"]
     exists=None
     if kw.has_key("exists"): exists=kw["exists"]
     vector=None 
     if kw.has_key("vector"): vector=kw["vector"]
     space_only=None
     if kw.has_key("space_only"): space_only=kw["space_only"]
     kw["remove"]=None
     kw["clean"]=None

     tag=None

     SEP.vec_sfloat.vector.__init__(self,name,tag,vector,space_only=1)


     if vector:
       cc=SEP.pv_func.clean_vec_keywords(vector.kw,["space_only","load"])
       cc["name"]=name
       for d,v in kw.items(): cc[d]=v
       SEP.pf_split.parfile.__init__(self,**cc)
       self.sect_map=vector.sect_map
     else:
       kw["remove"]=None
       kw["collect_it"]=None
       SEP.pf_split.parfile.__init__(self,**kw)
       if kw.has_key("load") and kw["load"]: 
         self.restart(1)
         kw["sect_map"]=self.get_stat_elem(1)
       if kw.has_key("sect_map"): self.sect_map=kw["sect_map"]
       else: SEP.util.err("Must provide a dictionary key:machine")
     if exists:
       a=self.sepfile_from_sects()
       for iax in range(a.ndims()): 
         n,o,d,label,unit=a.axis(iax+1)
         self.set_axis(iax+1,n,o,d,label,unit)
     

  def load(self):
    """LOad a parallel vector"""
    SEP.vec_sfloat.vector.load(self)
    self.restart(1)
    self._unlock_it()
  
  def redo(self,key,redo=None):
     """Mark that a section (key)  needs to be redone"""
     self.update_status(key,["todo"])

  def init_file(self):
    """Init the file"""
    self.restart(1)  #we just load the file

  def size(self):
    """Return the size (faked)"""
    return 1


  def dot(self,othervec):
     """Dot product vector with  another vector """
     if self.vec_name()==othervec.vec_name():
       error=self.solver_func("op=dot")
     else:
       error=self.solver_func("op=dot",othervec)
     r=0.0
     b=re.compile("\s*DOT\s+RESULT\s+(.+)\s+:")
     for key in self.return_keys():
       f=open(SEP.log.logging.old_file_name("solver_func.%s"%key))
       a=b.search(string.join(f.readlines()))
       if not a: SEP.util.error("Trouble with dot product")
       else: 
          r=r+float(a.group(1))
       f.close()
     return complex(r,0.0)



  def solver_func(self,args,file2=None,restart=None):
    """Run a mathemattical operation using Solvver_ops
    
       args     - a list of arguments for Math
   
    """
    a=solver_op_par("solver_func",args,self,file2,restart)
  def create_data(self):
     """out=0."""
     if len(self.nblock)!=1 : SEP.util.err("Only work when data is split along a single axis")
     iax=self.dff_axis[0]
     nblock=int(self.nblock[0])
     temp_file=SEP.util.temp_file("tmp")
     for key,mlabel in self.sect_map.items():
       sect_out,sect_out_new=self._section_name(key,mlabel)
       self.record_section(key,mlabel,sect_out)
       self.done(key)
       sepS=SEP.sepfile.sep_file(sepfile=self,name=temp_file)
       n,o,d,label,unit=sepS.axis(iax)
       n,o,d=dff_part(int(key),nblock,n,o,d)
       sepS.set_axis(iax,n,o,d,label,unit)
       sepS.history.add_param("esize",8)
       sepS.history.add_param("in","-1")
       sepS.write_file()
       mach=SEP.mach.mach_from_label(mlabel)
       SEP.spawn.run_wait(SEP.rc.cp_to(mach,temp_file,sect_out),oper_len=200,ntrys=3)
     SEP.spawn.run_wait("rm %s"%temp_file,ntrys=3)
     self.solver_func("op=zero")
     self.space_only=None
     self._unlock_it()

  def clean(self):
    SEP.pf_split.parfile.clean_parts()
    

  def clone(self,**kw):
    """Clone a parallel file object (split)"""
    kout=SEP.pv_func.clean_vec_keywords(SEP.pv_func.combine_keywords(self.kw,kw),["load","exists"])
    x=SEP.pv_func.combine_keywords(self.kw,kw)
    if not kout.has_key("name") or not kout["name"]: kout["name"]="%s.copy"%self.vec_name()
    kout["vector"]=self
    vec=float_vector(**kout)
    if not kout.has_key("space_only")  or not  kout["space_only"]:
      for key in self.return_keys():
        mlabel=self.get_key_status_elem(key,1)
        sect_in=self.get_key_status_elem(key,2)
        sect_out,sect_out_new=vec._section_name(key,mlabel)
        vec.record_section(key,mlabel,sect_out)
        mach= SEP.mach.mach_from_label(mlabel)
        SEP.spawn.run_wait(SEP.rc.remote_com(mach,"%s/Scp3d %s %s "%(
         SEP.paths.sepbindir,sect_in,sect_out)),ntrys=3)
    return vec

class cmplx_vector(SEP.pf_split.parfile,SEP.vec_scmplx.vector):
  def __init__(self,**kw):
     """Initialize a complex parallel vector

        Requred:
           name  - The name of the dataset 

        Optional parameters:
          exists - Whether or not the datast exists (assumed no [None])

          vector - Another vector

          sect_map - Map relating key to the machine it should be on

          space_only - (defaults to None) means we are only describing the 
           vector space (not necessarily any data associated with it)

     """
     if not kw.has_key("name"): SEP.util.err("Must provide the name of the dataset")
     name=kw["name"]
     exists=None
     if kw.has_key("exists"): exists=kw["exists"]
     vector=None 
     if kw.has_key("vector"): vector=kw["vector"]
     space_only=None
     if kw.has_key("space_only"): space_only=kw["space_only"]
     kw["remove"]=None
     kw["clean"]=None

     tag=None

     SEP.vec_scmplx.vector.__init__(self,name,tag,vector,space_only=1)


     if vector:
       cc=SEP.pv_func.clean_vec_keywords(vector.kw,["space_only","load"])
       cc["name"]=name
       for d,v in kw.items(): cc[d]=v
       SEP.pf_split.parfile.__init__(self,**cc)
       self.sect_map=vector.sect_map
     else:
       kw["remove"]=None
       kw["collect_it"]=None
       SEP.pf_split.parfile.__init__(self,**kw)
       if kw.has_key("load") and kw["load"]: 
         self.restart(1)
         kw["sect_map"]=self.get_stat_elem(1)
       if kw.has_key("sect_map"): self.sect_map=kw["sect_map"]
       else: SEP.util.err("Must provide a dictionary key:machine")
     if exists:
       a=self.sepfile_from_sects()
       for iax in range(a.ndims()): 
         n,o,d,label,unit=a.axis(iax+1)
         self.set_axis(iax+1,n,o,d,label,unit)
     

  def load(self):
    """LOad a parallel vector"""
    SEP.vec_scmplx.vector.load(self)
    self.restart(1)
    self._unlock_it()
  
  def redo(self,key,redo=None):
     """Mark that a section (key)  needs to be redone"""
     self.update_status(key,["todo"])

  def init_file(self):
    """Init the file"""
#   self._unlock_it()
    self.restart(1)  #we just load the file

  def size(self):
    """Return the size (faked)"""
    return 1

  def dot(self,othervec):
     """Dot product vector with  another vector """
     if self.vec_name()==othervec.vec_name():
       error=self.solver_func("op=dot")
     else:
       error=self.solver_func("op=dot",othervec)
     r=0.0
     i=0.0
     b=re.compile("\s*DOT\s+RESULT\s*\(\s*(.+)\s*,\s*(.+)\s*\)")
     for key in self.return_keys():
       f=open(SEP.log.logging.old_file_name("solver_func.%s"%key))
       a=b.search(string.join(f.readlines()))
       if not a: SEP.util.error("Trouble with dot product")
       else: 
          r=r+float(a.group(1))
          i=i+float(a.group(2))
       f.close()
       sys.stdout.flush()
     b=complex(r,i)
     sys.stdout.flush()
     return b


#  def solver_func(self,file1,args,file2=None,restart=None):
#    """Run a mathemattical operation using Solvver_ops
#    
#       args     - a list of arguments for Math
##   
#    """
#    a=solver_op_par("solver_func",args,file1,file2,restart)
#

  def solver_func(self,args,file2=None,restart=None):
    """Run a mathemattical operation using Solvver_ops
    
       args     - a list of arguments for Math
   
    """
    a=solver_op_par("solver_func",args,self,file2,restart)
  def create_data(self):
     """out=0."""
     if len(self.nblock)!=1 : SEP.util.err("Only work when data is split along a single axis")
     iax=self.dff_axis[0]
     nblock=int(self.nblock[0])
     temp_file=SEP.util.temp_file("tmp")
     for key,mlabel in self.sect_map.items():
       sect_out,sect_out_new=self._section_name(key,mlabel)
       self.record_section(key,mlabel,sect_out)
       self.done(key)
       sepS=SEP.sepfile.sep_file(sepfile=self,name=temp_file)
       n,o,d,label,unit=sepS.axis(iax)
       n,o,d=dff_part(int(key),nblock,n,o,d)
       sepS.set_axis(iax,n,o,d,label,unit)
       sepS.history.add_param("esize",8)
       sepS.history.add_param("in","-1")
       sepS.write_file()
       mach=SEP.mach.mach_from_label(mlabel)
       SEP.spawn.run_wait(SEP.rc.cp_to(mach,temp_file,sect_out),oper_len=200,ntrys=3)
     SEP.spawn.run_wait("rm %s"%temp_file,ntrys=3)
     self._unlock_it()
     self.solver_func("op=zero")
     self.space_only=None


  def clean(self):
    SEP.pf_split.parfile.clean_parts()
    
  def clone(self,**kw):
    """Clone a parallel file object (split)"""
    kout=SEP.pv_func.clean_vec_keywords(SEP.pv_func.combine_keywords(self.kw,kw),["load","exists"])
    x=SEP.pv_func.combine_keywords(self.kw,kw)
    if not kout.has_key("name") or not kout["name"]: kout["name"]="%s.copy"%self.vec_name()
    kout["vector"]=self
    vec=cmplx_vector(**kout)
    if not kout.has_key("space_only")  or not  kout["space_only"]:
      for key in self.return_keys():
        mlabel=self.get_key_status_elem(key,1)
        sect_in=self.get_key_status_elem(key,2)
        sect_out,sect_out_new=vec._section_name(key,mlabel)
        vec.record_section(key,mlabel,sect_out)
        mach= SEP.mach.mach_from_label(mlabel)
        SEP.spawn.run_wait(SEP.rc.remote_com(mach,"%s/Scp3d %s %s "%(
         SEP.paths.sepbindir,sect_in,sect_out)),ntrys=3)
    return vec

def dff_part(ipart,nblock,n,o,d):
  nsmall=int(n/nblock)
  nextra=n-nblock*nsmall
  ifirst=ipart*nsmall
  if ipart >= nextra: 
    n=nsmall; o=o+d*(ifirst+nextra)
  else:
    n=nsmall+1; o=o+d*(ifirst+ipart)
  return n,o,d


class solver_op_par(SEP.pj_split.par_job):
  def __init__(self,name,args,file1,file2=None,restart=None):
    """Initialize a solver operation parallel job"""     
    arg=SEP.args.basic(name="ndsf")
    if type(args) != types.ListType:args=args.split()
    args.append("verb=y")
    for a in args: arg.add_string(a)
    SEP.pj_split.par_job.__init__(self,name)
    self.add_param("program","%s/Solver_ops"%SEP.paths.sepbindir)
    self.add_param("assign_map",file1.get_stat_elem(1))
    files={}
    files["file1"]=file1
    f1s=files["file1"].kw["tag"] 
    files["file1"].kw["tag"]="file1="
    if file2:
      files["file2"]=file2
      f2s=files["file2"].kw["tag"] 
      files["file2"].kw["tag"]="file2="
    self.add_param("files",files)
    self.add_param("global_pars",arg)
    self.add_param("sect_pars",SEP.pj_func.simple_parameter_list(len(file1.return_keys())))
    pp=SEP.opt_data.find("parallel").return_set_params()
    self.set_cl_par_params(pp)
    self.add_param("device",pp.param("device"))
    SEP.pj_split.par_job.prep(self,restart)
    self.set_cl_par_params(SEP.opt_data.find("parallel").return_set_params())
    SEP.pj_split.par_job.run(self,restart)
    self.clean_files()
    files["file1"].kw["tag"]=f1s
    if file2: files["file2"].kw["tag"]=f2s


