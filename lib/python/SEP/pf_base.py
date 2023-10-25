import SEP.sep_socket
import SEP.stat_sep
import SEP.sepfile
import os,sys,types,commands,string,re
import SEP.util
import SEP.datapath
import SEP.paths
import SEP.pc
import SEP.rc
import SEP.mach
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.7.30"
                                                                                


 
class parfile(SEP.sepfile.sep_file,SEP.stat_sep.status):
  """ Define a sep parallel file"""
  def __init__(self,**kw):
     """Initialize a parfile object

        Required parameters:
        name        -   The name of the combined file
        tag         -   The tag associated with the file (e.g. data=)
        usage       -   Usage for file (INPUT or OUTPUT)

        Optional parameters:
        njobs       -   Total number of jobs
        load        -   Whether or not [default] to load the file 
        verb        -   Verbosity level (None)
        add         -   Whether or not we are adding to the output (None)
        reuse_par   -   Parameter when a file is being reused (defaults to none)
        remove      -   Whether or not use remove the part files (defaults to yes)
        space_only  -   Whether or not we are just creating the space (defaults to no)
        collect_it  -   Whether or not collect the file
   
     """
 
     self.type=None
     self.kw=kw
     self.file_name=self._option("name",err=1)  
     self.usage=self._option("usage")
     self.stat_file=self._option("stat_file")
     self.stat_file="%s.stat"%os.path.basename(self.file_name)
     self.mach_func=None



     self.add_to=self._option("add",None)
     self.verb=self._option("verb")
     self.reuse_par=self._option("reuse_par","add=y")
     self.space_only=self._option("space_only")
     self.remove=self._option("remove",default=1)
     self.collect_it=self._option("collect_it",default=1)

     self.loadit=self._option("load",default=None)

     if self.loadit:
       opts=self.load_stat(self.stat_file)
       for key,val in opts.items(): self.kw[key]=val
     else: 
       self.isect_dict=SEP.pj_func.simple_section_list(int(self._option("njobs",err=1)))

     SEP.stat_sep.status.__init__(self,self.stat_file,
       self.isect_dict.keys(),njob_extra=2)

     self.add_file_param("nsect",len(self.isect_dict.keys()))

  def init_file(self):
    """Initialize the file"""
    self.init_all()

  def load_stat(self,name):
    """Load a status file from disk"""
    aa=SEP.stat_sep.status(name,load=1)
    pars=aa.pars.return_set_params_dict()
    self.isect_dict={}
    for k in aa.return_keys(): self.isect_dict[k]=int(k)
    self.kw["njobs"]=len(self.isect_dict.keys())
    return pars

  def _section_name(self,key,mlabel):
    """Return the section name given the key and machine label (must override)"""
    SEP.util.err("Section name must be overriden")

  def clone(self,**kw):
    """Clone a parallel file object (must override)"""
    SEP.util.err("Clone must be overriden")


  def mach(self):
    """Return the machine class associated with the parallel file"""
    if not self.mach_func: 
      if not SEP.mach.mach:  SEP.util.err("Machine function not setup")
      self.mach_func=SEP.mach.mach
    return self.mach_func

  def record(self,key):
     """Record that a job(key) has started correctly"""
     self.set_key_status_elem(key,0,"running")

  def done(self,key):
     """Mark that a section (key) has been processed"""
     self.set_key_status_elem(key,0,"finished")

  def redo(self,key,redo=None):
     """Mark that a section (key)  needs to be redone"""
     self.update_status(key,["todo","none","none"])

  def nsect(self):
    """Return the number of sections"""
    return len(self.isect_dict.keys())

  def tags(self,key_label,logfile):
    """Get local section names

        key_label is a dictionry linking key names and machine labels
        Return is a dicitonary linking key names and tags

        If the file is  "INPUT" the data will be distributed to the nodes if needed
        If the file is being reused or restarted appropriate parameters will be added

    """
    err,sect_names,new_list=self._get_section_names(key_label,logfile)
    if err: return err,None
    tag_list={}
    self.tag=self._option("tag",err=1)
    for key in sect_names.keys():
      tag_list[key]=self.tag+sect_names[key]
      if self.usage =="OUTPUT":
        tag_list[key]+="  datapath=%s"%self._datapath(SEP.mach.mach_from_label(key_label[key]))
        tag_list=self.add_reuse(key,new_list,tag_list)
    return None,tag_list


  def add_reuse(self,key,new_list,tag_list):
    if self.reuse_par:
      if 0== new_list.keys().count(key):
        tag_list[key]+=" "+self.reuse_par
    return tag_list

  def _datapath(self,mach):
    """Return the datapath for the output file"""
    return SEP.datapath.datapath(mach)

  def restart(self,restart=None,finished_list=["finished","collected"],restart_list=["running"],reinit_list=["sent"]):
    if self.loadit: restart=1
    SEP.stat_sep.status.restart(self,restart,finished_list,restart_list,reinit_list)
    

  def _get_section_names(self,key_label,logfile):
    """Get the section names given the key list
        
       Given key:machine label returns the section names

    """
    new_mach={}
    new_sect={}
    section_list={}
    for key,mlabel in key_label.items():
      a,b=self._section_name(key,mlabel)
      section_list[key]=a
      if b: 
        new_mach[key]=SEP.mach.mach_from_label(mlabel)
        new_sect[key]=a
      self.record_section(key,mlabel,a)
      if self.verb >4: SEP.util.msg("tag=%s '%s' isect=%d section=%s machine=%s"%(self.file_name,key,self.isect_dict[key],a,mlabel))
    if new_mach.keys():
       self._remove_old_sects(new_mach,new_sect)
       if self.usage=="INPUT": 
         if self._partition(new_mach,new_sect,logfile): return 1,None,None
       self._unlock_it()
    return None,section_list,new_mach
   
  def _remove_old_sects(self,key_mach,key_sect):
    """Remove old sections 

        Given key:mach and key:section removes old versions of the files

    """
    for key in key_mach.keys():
       SEP.spawn.run_wait(SEP.rc.remote_com(key_mach[key],
         "rm -f %s"%key_sect[key]), 60, error=None,quiet=1,ntrys=3)
       SEP.spawn.run_wait(SEP.rc.remote_com(key_mach[key],
         "rm -f %s@*"%key_sect[key]), 60, error=None,quiet=1,ntrys=3)

  def _keys_from_machine(self,mach):
    """Return all of the sections from a given machine"""
    label_list=self.mach().label_list(mach)
    sect_on_mach=[]
    for label in label_list: sect_on_mach.extend(self.get_elem_val(1,label))
    return sect_on_mach
        

  def record_section(self,key,label,section):
     """Record to the status file that we have set the machinename"""
     self.update_status(key,["sent",label,section,"none"])


  def set_collect(self,collect_val):
    """Set whether or not to collect"""
    self.collect=collect_val

  def finish(self):
    """Finish processing a file"""
    if self.usage=="OUTPUT" and self.collect_it: 
      self.collect(self.isect_dict.keys())
    
  def _option(self,par,default=None,kw=None,err=None):
    """Return an optional paramter if it exists, else None"""
    if kw==None: kw=self.kw
    if not kw.has_key(par):
      if err: SEP.util.err("Missing required parameter %s"%par)
      return default
    return self.kw[par]

  def _partition_pars(self,key_mach,key_sect):
    """Return the command line parameters for distributing a dataset

       key_mach a dictionary key:machine
       key_sect a dictionary key:section name

       returns a list that is command line parameters to partition the dataset

    """
    SEP.util.err("Must override partition pars")

  def _collect_pars(self,key_mach,key_sect):
    """Return the command line parameters for collecting a dataset

       key_mach a dictionary key:machine
       key_sect a dictionary key:section name

       returns a list that is command line parameters to collect the dataset

    """
    SEP.util.err("Must override collect pars")

  def _partition(self,key_mach,key_sect,logfile):
     """Partition the dataset"""
     if self.usage != "INPUT": SEP.util.err("Can only partition INPUT")
     pars,machs,nbytes=self._partition_pars(key_mach,key_sect)
     if self.verb >1: SEP.util.msg("Distributing %s to %s"%(self.file_name,
         string.join(key_mach.values())))
     return SEP.pc.run_job(pars,machs,logfile,nbytes)[0]

  def collect(self,key_list,logfile,extra_args=[" "]):
     """Collect the dataset"""
     print "WHAT THE 1",self.file_name,self.usage
     if self.usage != "OUTPUT" or not self.collect_it: return
     pars,machs,nbytes=self._collect_pars(key_list)
     pars.extend(extra_args)
     if self.verb >1: SEP.util.msg("Collecting  %s from %s"%(self.file_name,
        string.join(machs)))
     return SEP.pc.run_job(pars,machs,logfile,nbytes)[0]

  def _remove_parts(self,keys=None):
    """Remove the sections"""
    if  not keys:keys=self.return_keys()
    if not self.remove: return
    mach_rm={}
    for key in keys:
      machlabel=self.get_key_status_elem(key,1)
      if machlabel!="none":
        mach= SEP.mach.mach_from_label(machlabel)
        if not mach_rm.has_key(mach): mach_rm[mach]=[]
        tag= self.get_key_status_elem(key,2)
        if tag != "none":
          tfile=SEP.sepfile.sep_file(remote="%s:%s"%(mach,tag))
          mach_rm[mach].append(tag)           #remove the history file
          mach_rm[mach].append(tfile.history.param("in"))  #remove the data binary
          if tfile.headers:
            mach_rm[mach].append(tfile.history.param("hff"))  #remove the hff
            mach_rm[mach].append(tfile.headers.param("in"))  #remove the headers binary
          if tfile.grid:
            mach_rm[mach].append(tfile.history.param("gff"))  #remove the grid file
            mach_rm[mach].append(tfile.grid.param("in"))  #remove the grid binary

    icount=0
    for mach,tags in mach_rm.items():
      icount+=1 
      if icount==6:
        icount=0; 
        SEP.spawn.run_wait(SEP.rc.remote_com(mach,
         "rm -f %s"%string.join(tags)), 60, error=None,quiet=1,ntrys=3)
      else:
        SEP.spawn.run_wait(SEP.rc.remote_com(mach,
         "rm -f %s&"%string.join(tags)), 60, error=None,quiet=1,ntrys=3)

#  def axis(self,iaxis):
#    """Return an axis in the dataset"""
#    SEP.util.err("Axis must be overridden")

  def sect_sepfile(self,key):
    """Return the sepfile description of a remote element of the dataset"""
    sect=self.get_key_status_elem(key,2)
    mach=SEP.mach.mach_from_label(self.get_key_status_elem(key,1))
    if sect=="none":  SEP.util.err("Can't retrieve an unitialized remote file key=%s "%key)
    return SEP.sepfile.sep_file(remote="%s:%s"%(mach,sect))

  def sepfile_from_sects(self):
    """Return the sepfile related to the entire dataset"""
    SEP.util.err("Must override")


  def clean_parts(self):
    """Clean all the intermediate portions"""
    if self.remove: 
      self._remove_parts()
      SEP.stat_sep.status.clean(self)
    dirs=["."]
    ab=re.compile("^[\.\w _\-@:~]+$")
    for dir in dirs:
      for file in os.listdir(dir):
        filep=dir+"/%s"%file
        st=os.stat(filep)
        if st.st_size==0 and not ab.search(file):
          os.unlink(filep)

