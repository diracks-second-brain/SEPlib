import os
import sys
import string
import SEP.args
import SEP.util
import SEP.opt_none
import SEP.spawn
import types
import SEP.stat_sep

__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.5.29"
print_flow=None

class flow(SEP.opt_none.options):
  """A class for a combination of different modules [e.g. programs]"""
  def __init__(self,name,par_blocks=[],prefixes={},flows=[],restartable=None,order=[]):
    """A Class for a series of modules"""
    self.my_name=name
    self.par_blocks= par_blocks
    self.flows=flows
    self.prefixes=prefixes
    self.order=order
    self.restartable=restartable
    SEP.opt_none.options.__init__(self,name)
    self.set_prefixes()

                                                                                
  def run_in_flow(self):
     return 1


  def set_prefixes(self,prefix=""):
    order,op_dict,type_dict=self.return_job_order()
    self.set_prefix(prefix)
    for o in order:
      if not type_dict.has_key(o):
        SEP.util.err("Structure %s doesn't exist"%o)
      if type_dict[o][0]=="F":
        if self.prefixes.has_key(op_dict[o].my_name): pre=self.prefixes[op_dict[o].my_name]
        else: pre=""
        op_dict[o].set_prefixes("%s%s"%(prefix,pre))
      else:
        name=op_dict[o].return_name()
        if self.prefixes.has_key(name): pfx="%s%s"%(prefix,self.prefixes[name])
        else: pfx=prefix
        op_dict[o].set_prefix(pfx)

  
  def get_options(self,par_dict,args,delete=None):
    """Return the options for this flow"""

    err,args=self.read_params(args,delete)
    if err: SEP.util.err("Parameter problem in %s (1)"%name)
    order,op_dict,type_dict=self.return_job_order()
    for o in order:
      if not type_dict.has_key(o):
        SEP.util.err("Structure %s doesn't exist"%o)
      if type_dict[o][0]=="F":
        par_dict,args=op_dict[o].get_options(par_dict,args,delete)
      else:
        name=op_dict[o].return_name()
        err,args=op_dict[o].read_params(args,delete)
        if err: SEP.util.err("Parameter problem in %s (2)"%name)
        op_dict[o].build_check_params()
        par_dict[name]=op_dict[o]
    return  par_dict,args

  def return_name(self):
    """Return the name for this parameter block"""
    return self.my_name

  def check_build_restartable(self,restart):
    """Build the list of commands to run"""
    restart_list=[]
    if self.restartable:
      job_list=self.return_job_list()
      self.status=SEP.stat_sep.status(self.my_name+".stat",job_list)
      restart_list=self.status.restart(restart,
           ["finished"],["started","restart"])
      return restart_list
    else: return None

  def return_job_order(self):
    """Return the order in which jobs should be run"""
    if not self.order:
      order=[]
      for f in self.flows: order.append(f.my_name)
      for p in self.par_blocks: order.append(p.my_name)
    else: order=self.order
    op_dict={}; type_dict={} #could be done with type function
    for f in self.flows: 
      op_dict[f.my_name]=f
      type_dict[f.my_name]="FLOW"
    for p in self.par_blocks: 
      op_dict[p.my_name]=p
      type_dict[p.my_name]="PAR_GROUP"
    return order,op_dict,type_dict

  def get_name(self):
    """The name of the job to run"""
    return self.my_name
  
  def return_job_list(self):
    """Return the list of jobs for this job"""
    job_list=[]
    for f in self.flows: job_list.append(f.get_name())
    for block in self.par_blocks: job_list.append(block.return_name())
    return job_list
    
  def add_options(self,p,par_dict,args,delete):
    """Add options for the given flow"""
    for block in self.par_blocks:
       name=block.return_name()
       err,args=block.read_params(args,delete)
       if err: SEP.util.err("Parameter problem in %s"%block.my_name)
       block.build_check_params()
       par_dict[name]=block
    return  par_dict,args
  def add_par_block(self,par):
    """Add a parameter block to the program"""
    self.par_blocks.append(par)

  def  prep_run(self,restart=None):
    """Run the job(s) comprising the program"""
    restartit=self.check_build_restartable(restart)
    order,op_dict,type_dict=self.return_job_order()
    for o in order:
      if  not op_dict.has_key(o):
        SEP.util.err("Trouble finding %s doesn't exist in flows or par groups"%o)
      doit=1
      restart=None
      if self.restartable: 
         if restartit.count(o)==1: 
           restart=1
         if  self.status.get_key_status_elem(o,0)[0:8]=="finished": doit=None
      if doit:
        if op_dict[o].run_in_flow():
          if self.restartable: self.status.update_status(o,["started"],lock=1)
          op_dict[o].prep_run(restart) 
          op_dict[o].clean_files()
          if self.restartable: self.status.update_status(o,["finished"],lock=1)

  def clean_files(self):
    """Clean any files from the job"""
    if os.path.isfile("rm %s.stat"%self.my_name):
      SEP.spawn.run_wait("rm %s.stat"%self.my_name)


  def check_build_order(self):
    """Build the order in which jobs should be run"""


  def doc_pars(self):
    """Return the docs for a given parameter class"""
    ln=self.return_doc()
    if ln:
      lines=[self.name]
      lines.extend(ln)
    else: lines=[]
    
    order,op_dict,type_dict=self.return_job_order()
    for o in order:
      if not type_dict.has_key(o):
        SEP.util.err("Structure %s doesn't exist"%o)
      if type_dict[o][0]=="F":
        lines.extend(op_dict[o].doc_pars())
      else:
        name=op_dict[o].return_name()
        b=op_dict[o].return_doc()
        if b:
          lines.extend(["  %s"%op_dict[o].return_name()])
          lines.extend(b)
    return lines
    
