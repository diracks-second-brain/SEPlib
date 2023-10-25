import SEP.pj_base
import os,sys,string,time,commands,signal,types,re,pwd
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.04.8"
                                                                                
class par_job(SEP.pj_base.par_job):
  """ A parallel job object where we are sharing image files"""

  def __init__(self,name="PAR"):
    """Initialize a parallel job object where we sharing image files

       SEE SEP.pj_base.parjob

    """
    SEP.pj_base.par_job.__init__(self,name)

  def  prep(self,restart=None):
    """Add additional parameters and check to see if the parameters make sense"""
    self.assign_map=self.param("assign_map",error=1)
    SEP.pj_base.par_job.prep(self,restart)

     
  def add_doc_params(self):
    """Add the required files for a nfs paralllel job"""
    SEP.pj_base.par_job.add_doc_params(self)
#    self.add_doc_param("assign_map",doc="Dictionary key=jobid value=mlabel")

  def assign_jobs(self):
     """Return the list of jobs to assign to each node using
         the defined assign map  """
     todo_list=self.jobs_to_do()
     mach_send={}
     if self.multi_start:
       machlabel=self.mach.free_labels()  #get list of machines we can run on
     else:
       machlabel=self.mach.free_mach()  #get list of machines we can run on
     mach_send={}
     mlabel_mach={}
     for label in machlabel: mlabel_mach[label]=self.mach.mach_from_label(label)
     
     #loop through all the jobs
     for key in todo_list:
       mach_to=self.mach.mach_from_label(self.assign_map[key])
       if 0!= mlabel_mach.values().count(mach_to):
         assigned=None
         for label,mach in mlabel_mach.items():
           if mach == mach_to and not assigned: 
             assign=label; del mlabel_mach[label]
         mach_send[key]=assign
     if not mach_send: self.check_die_nicely()
     return mach_send
  def check_die_nicely(self):
    """If we are relying a node that is dead report that we are waiting on
       it or die if we can't do any more work because of it"""

    todo_list=self.jobs_to_do()
    need_dead={}
    need_not_listed={}
    all_mlabels=self.mach.return_keys()
    all_machs=[]
    mlabel_exists={}
    for mlabel in all_mlabels: 
      mach=self.mach.mach_from_label(mlabel)
      if all_machs.count(mach)==0: all_machs.append(mach)
      mlabel_exists[mach]=mlabel
    cando_more=None
    for key in todo_list:
      mach_to=self.mach.mach_from_label(self.assign_map[key])
      if all_machs.count(mach_to)==0: need_not_listed[mach_to]=1
      elif self.mach.get_key_status_elem(mlabel_exists[mach_to],0)=="dead":
         need_dead[mach_to]=1
      else: cando_more=1
    if need_dead or need_not_listed:
      self.check_nodes("dead")
      SEP.util.msg("Nodes not listed:%s  \nNodes dead:%s"%(string.join(need_not_listed.keys()),string.join(need_dead.keys())))
      
    if not cando_more: 
      self.check_nodes("dead")
      if  self.get_by_status("todo",keys=todo_list) and self.mach.get_by_status("dead"):
        self.exit_clean()
