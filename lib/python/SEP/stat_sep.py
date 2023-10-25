import commands
import SEP.util
import os,string,fcntl,re,types
import SEP.stat_base
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.4.5"
                                                                                


class  status(SEP.stat_base.status):
  """Class where status  us stored as a : seperated list"""

  def __init__(self,file,job_list=None,njob_extra=0,order=None,load=None):
      """Intialize a sep_status object

          file         is the file to write the status to
          job_list     is the list of jobs
          njob_extra   is the number of elements (beyond 1) in the status
          order        is the order of the jobs

      """
      self.njob_extra=njob_extra
      self.psrch=re.compile("FILE DESCRIPTOR:(.+)$")
      self.pars=SEP.args.basic(name=file)
      SEP.stat_base.status.__init__(self,file,job_list,order,load=load)

  def init_status(self,par):
    """Set the initial status for a job"""
    args=["todo"]
    for i in range(self.njob_extra): args.append("none")
    self.status_dict[par]=string.join(args,":")

  def add_file_param(self,par,val):
    """Add a paramter to the status file"""
    self.pars.add_param(par,val)
    
  def file_param(self,par,default=None):
    """Add a paramter to the status file"""
    return self.pars.param(par,default)

  def update_status(self,par,stats,lock=None):
    """Update the status of a given task

       Optional parameter lock to lock status file
 
       Example:
         stat.update_status('1',['todo','alpha','beta'])
         sets the status of '1' to 'todo:alpha,beta'
      
    """
    if lock: self._lock_it()
    the_status=self.status_dict[par].split(":")
    for i in range(len(stats)): 
       if i < len(the_status): the_status[i]=stats[i] 
       else: the_status.append(stats[i])
    self.status_dict[par]=string.join(the_status,":")
    self.update.append(par)
    if lock: self._unlock_it()

  def add_header(self,file):
    """Add extra info to the file"""
    list=self.pars.return_set_params_list(quote=None)
    for item in list:  
      file.write("FILE DESCRIPTOR: %s \n"%item)
  
  def line_to_status(self,dict,line):
    """Convert a line to internal status representation"""
    if self.psrch.search(line):
      self.pars.add_string(self.psrch.search(line).group(1))
    else: 
      ar=line.split(":")
      dict[ar[0]]=string.join(ar[1:],":")
    return dict

  def status_to_line(self,key,val):
    """Convert internal status to line"""
    a=[key]
    a.extend(val.split(":"))
    return "%s\n"%string.join(a,":")

  def restart(self,restart=None,finished_list=["finished","collected"],restart_list=["running"],reinit_list=["sent"]):
     """Setup a restart 

        restart       - whether or not we are restarting this job
        finished_list - statuses that signify a job is completed
        restart_list  - statuses that signify that the job was running
        reinit_list   - statuses that signify that the job was sent

        Returns the jobs that need to restarted
        
        The status of jobs that are reinit_list and restart_list are marked
        todo.finished_list are left unchanged

     """
     self._lock_it(read=restart)
     if restart: 
       my_list=[]
       restart=[]
       if finished_list: my_list.extend(finished_list)
       if restart_list: my_list.extend(restart_list)
       if reinit_list: my_list.extend(reinit_list)
       for key,val in self.status_dict.items():
         stats=val.split(":")
         oldstat=stats[0]
         if my_list and my_list.count(stats[0])==0: 
           self.update_status(key,["todo"])
         elif restart_list and restart_list.count(oldstat): 
           stats[0]="todo"
           self.update_status(key,stats)
           restart.append(key)
         elif reinit_list and reinit_list.count(oldstat): 
           self.init_status(key)
           self.update.append(key)
#           stats[0]="todo"
#           self.update_status(key,stats)
#           restart.append(key)
         else: 
           self.update.append(key)
     else: restart=[]
     self._unlock_it()
     return restart


  def print_status_info(self,job):
     """Print the status of a given job"""
     SEP.util.msg( "job[%s]=%s",job,self.status_dict[job])

  def get_stat_elem(self,elem,keys=None):
     """Get one element of status given a key list

        Get the ith (elem) value of the status of a set of keys.
        Defaults to all of the keys.
        Returns a dictionary of form key:elem value

        Example: 
          stat={0:'todo:a1:b1',1:'done:b2:a2',2:'sent:c3:e3',3:'todo:c4:e4'}
          stat.get_status_elem(0) returns
             0:todo,1:done,2:sent 
          stat.get_status_elem(1,keys=[1,2]) returns
             1:b2,2:b3

     """
     other_list={}
     if keys: key_list=keys
     else: key_list=self.order
     for key in key_list:
       other_list[key]=self.status_dict[key].split(":")[elem]
     return other_list

  def get_elem_val(self,elem,val,ntot=99999,keys=None):
      """Return all keys that have a given value
        
        elem   - element in the status to search
        val    - value to search for
        ntot   - maximum number of elements to return
        keys   - keys to search (defaults to all)

        Example: 
          stat={0:'todo:a1:b1',1:'done:b2:a2',2:'sent:c3:e3',3:'todo:a1:b1'}

        stat.get_elem_val(1,'a1') returns [0,3]
        stat.get_elem_val(1,'a1',1) returns [0]
        stat.get_elem_val(1,'a1',keys=[1,2,3]) returns [3]

      """
      nfind=0
      stat_list=[]
      if type(keys) !=types.NoneType : key_list=keys
      else: key_list=self.order
      for item in key_list:
        if elem > len(self.status_dict[item].split(":")): 
           SEP.util.msg( self.status_dict[item])
        if self.status_dict[item].split(":")[elem]==val:
          stat_list.append(item)
          nfind+=1
          if nfind == ntot : break
      return stat_list

  def get_by_status(self,stat,ntot=9999999,keys=None):
      """Get up to ntot objects that have the status stat

        val    - value to search for
        ntot   - maximum number of elements to return
        keys   - keys to search (defaults to all)

        Example: 
          stat={0:'todo:a1:b1',1:'done:b2:a2',2:'sent:c3:e3',3:'todo:a1:b1'}

        stat.get_by_status('todo') returns [0,3]
        stat.get_by_status('todo',1) returns [0]
        stat.get_by_status('todo',keys=[1,2,3]) returns [3]

      """
      return self.get_elem_val(0,stat,ntot,keys)
   
  def elem_not_list(self,not_list,elem,ntot=999999,keys=None):
      """Return the list of keys that don't have a set of statuses

        not_list - values to search for
        elem   - element in the status to search
        ntot   - maximum number of elements to return
        keys   - keys to search (defaults to all)

        Example: 
          stat={0:'todo:a1:b1',1:'done:b2:a2',2:'sent:c3:e3',3:'todo:a1:b1'}

        stat.elem_not_list(['todo'],0) returns [1,2]
        stat.elem_not_list(['todo'],0,1) returns [1]
        stat.elem_not_list(['sent'],0,keys=[1,2,3]) returns [0,1,3]

      """
      if keys: key_list=keys
      else: key_list=self.order
      ncount=0
      out_list=[]
      for item in key_list:
        if 0==not_list.count(self.status_dict[item].split(":")[elem]):
          out_list.append(item)
          ncount=ncount+1
          if ncount==ntot: return out_list
      return out_list

  def get_status(self,key):
    """Get the status of a given key (returns it as a list)"""
    key=str(key)
    if self.status_dict.has_key(key): return self.status_dict[key].split(":")
    SEP.util.err("Asking for status key %s that doesn't exist"%key)

  def get_key_status_elem(self,key,num):
    """Get a specific status element

        Example: 
          stat={0:'todo:a1:b1',1:'done:b2:a2',2:'sent:c3:e3',3:'todo:a1:b1'}
          stat.get_key_status_elem('2', 2) returns 'e3'

    """
    a=self.get_status(key)
    if len(a) <= int(num): return None
    return a[int(num)]

  def set_key_status_elem(self,key,num,val):
     """Set a status element

        Example: 
          stat={0:'todo:a1:b1',1:'done:b2:a2',2:'sent:c3:e3',3:'todo:a1:b1'}
          stat.set_key_status_elem('2', 2,'b4') 
          stat={0:'todo:a1:b1',1:'done:b2:a2',2:'sent:c3:b4',3:'todo:a1:b1'}

     """
     if not self.status_dict.has_key(key):
       SEP.util.err("'%s' does not exist in status file"%key)
     ar=self.status_dict[key].split(":")
     if len(ar) < num+1:
       for i in range(len(ar),num+1): ar.append(0)
     ar[num]=str(val)
     self.status_dict[key]=string.join(ar,":")
     self.update.append(key)

  def clean(self):
    """Clean up"""
    SEP.spawn.run_wait(["rm",self.file])

  def status_not(self,not_list,ntot=99999,keys=None):
     """Return all status not equal to values in not List

        not_list - values to search for
        ntot   - maximum number of elements to return
        keys   - keys to search (defaults to all)

        Example: 
          stat={0:'todo:a1:b1',1:'done:b2:a2',2:'sent:c3:e3',3:'todo:a1:b1'}

        stat.elem_not_list(['todo'], returns [1,2]
        stat.elem_not_list(['todo'],1) returns [1]
        stat.elem_not_list(['sent'],keys=[1,2,3]) returns [0,1,3]

     """
     return self.elem_not_list(not_list,0,ntot,keys)
