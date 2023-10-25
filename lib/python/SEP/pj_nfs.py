import SEP.pj_base
import os,sys,string,time,commands,signal,types,re,pwd
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.03.18"
                                                                                
class par_job(SEP.pj_base.par_job):
  """ A parallel job object where we are sharing image files"""

  def __init__(self,name="PAR"):
    """Initialize a parallel job object where we sharing image files

       SEE SEP.pj_base.par_job

    """
    SEP.pj_base.par_job.__init__(self,name)

  def  prep_run(self,restart=None):
    """Add additional parameters and check to see if the parameters make sense"""
    self.registered=None
    self.kw['redo']=None
    SEP.pj_base.par_job.prep_run(self,restart)
     
  def add_doc_params(self):
    """Add the required files for a nfs paralllel job"""
    SEP.pj_base.par_job.add_doc_param()
    self.del_doc_param("program")
    self.del_doc_param("redo")

  def assign_jobs(self):
     """Return the list of jobs to assign to each node 
        In this case we will only allow one to be started at a time
        to make sure NFS read/write doesn't kill us"""
     mach_send=SEP.pj_base.par_job.assign_jobs(self)
     mach_={}
     if mach_send and not self.registered: 
       self.registered=1
       mach_[mach_send.keys()[0]]=mach_send.values()[0]
     return mach_
       

  def _record(self,jobid,machid,extra):
     """" Record that a task is running"""
     SEP.pj_base.par_job._record(self,jobid,machid,extra)
     self.registered=None
