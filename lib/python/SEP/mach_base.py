import time,re,os,commands,string,pwd
import SEP.spawn
import SEP.rc
import SEP.util
import SEP.datapath
import SEP.stat_sep
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.3.18"
                                                                                


class machine(SEP.stat_sep.status):
  """class containing list of potential machines to run on"""
                                                                                
  def __init__(self,nmax):
    """Initialize the machine database 
      nmax - Maximum number of jobs to run (2000)

    """
    self.machine_order={}
    self.machname=self.all_label_mach()    #mapping from machlabel:machine
    SEP.stat_sep.status.__init__(self,"job.mach",self.machname.keys(),njob_extra=2)
    self.labels=self.build_mach_label()
    self.nmax=nmax
    self._lock_it(read=None)
    self.write_status()
    self._unlock_it()

  def mach_order(self,list_in):
    """Return machines in predined order"""
    list_out=[]
    big=[] 
    for i in range(len(self.machine_order.keys())): big.append(None)
    for i in list_in: big[self.machine_order[i]]=i
    for i in big:
      if i: list_out.append(i)
    return list_out

  def build_mach_label(self):
    """Build mach:lael"""
    labels={}
    for key,val in self.machname.items():
      if labels.has_key(val): labels[val]+=":"+key
      else: labels[val]=key
    return labels

  def all_label_mach(self):
    """A function that maps label:mach"""
    SEP.util.err("")

  def init_status(self,par):
    """Set the initial status for a job"""
    args=["free"]
    for i in range(self.njob_extra): args.append("none")
    self.status_dict[par]=string.join(args,":")

  def init_list_file(self,machinefile):
    """Get the list of machines from a file"""
    try: f = open (machinefile)
    except:  SEP.util.err("Trouble opening file:"+machinefile)
    lines=f.readlines()
    nocommentS=re.compile('^(.+)\#')    #remove commented material
    machnS=re.compile('^(\S+):(\S+)')    #remove commented material
    machS=re.compile('^(\S+)')    #remove commented material
    machname={}
    nmach={}
    for line in lines:
       #remove the commented material
       nocomment=nocommentS.search(line.strip())
       if nocomment: do=nocomment[0]
       else: do=line.strip()
       ncount=0;
       machn=machnS.search(do)
       mach=machS.search(do)
       if machn:
         ncount=machn.group(2)
         mach=machn.group(1)
       if mach:
         ncount=1
         mach=mach.group(1)
       if nmach.has_key(mach): n=nmach[mach]
       else: n=0
       for i in range(ncount):
         name=mach+"-"+str(n)
         machname[name]=mach
         n+=1
       nmach[mach]=n
    f.close()
    return machname

  def mach_map(self):
    """return the mapping between machine descriptors and names"""
    return self.machname

  def mark_job_fail(self,jobid):
    """Mark that a job has failed therefore the machine is free"""
    mine=self.get_elem_val(1,jobid)
    if len(mine) !=1: 
      if len(mine) ==0: SEP.util.err("no jobs registerd on %s"%jobid)
      else: SEP.util.err("more than one  job registerd on %s:[%s] "%jobid,string.join(mine))
    self.update_status(mine[0],["free","none"])

  def mach_from_label(self,label):
     """Return the machine name given a label"""
     if not self.machname.has_key(label): 
       SEP.util.err("Label=%s not known"%label)
     return self.machname[label]

  def label_list(self,mach):
    """Return all of the labels for a given machine"""
    return self.labels[mach].split(":")

  def  not_dead(self):
    """Find a machine that isn't dead"""
    free_list=self.get_by_status("free",1)
    if free_list: return self.mach_from_label(free_list[0])
    free_list=self.get_by_status("running",1)
    if free_list: return self.mach_from_label(free_list[0])
    free_list=self.get_by_status("sent",1)
    if free_list: return self.mach_from_label(free_list[0])
    SEP.util.err("Can't find an active node")
    

  def free_labels(self):
    """Return the list of labels free"""
    return self.get_by_status("free",ntot=self.nmax) 


  def free_mach(self):
    """Return the list of free nodes """
    #ALL free nodes
    free_list=self.free_labels()

    #all the labels on these nodes
    label_list=[]
    for label in free_list:
      label_list.extend(self.label_list(self.mach_from_label(label)))

    #get the status for all the labels
    stat=self.get_stat_elem(0,keys=label_list)

    #find all nodes where we have a job started but not registed
    no_start=[]
    for key,val in stat.items():
       if val=="sent": no_start.extend(self.label_list(self.mach_from_label(key)))
 
    node_list=[]
    out_list=[]
    for free in free_list:
      if no_start.count(free) ==0 and node_list.count(self.mach_from_label(free))==0:
        node_list.append(self.mach_from_label(free))
        out_list.append(free)
    return out_list
        
      


  def machs_from_labels(self,labels):
    """Return a list of machines from their labels"""
    list=[]
    for i in labels: list.append(self.machname[i])
    SEP.util.condense(list)
    return list
  
  def start_problem(self,stat,longer):
    """Return jobs that have haven't been registered in a given time"""
    tm=time.time()-longer
    sent_list=self.get_by_status("sent")
    list=[]
    for mach in sent_list:
      mytm=self.get_key_status_elem(mach,2)
      if mytm < tm:  list.append(mach)



def check_alive(mach):
  """Check to see if the machine mach is alive"""

  err=None
  logfile=SEP.util.temp_file()
  ext="%s.%s"%(pwd.getpwuid(os.getuid())[0],os.getpid())
  file=" %sjdsfjkasd"%SEP.datapath.datapath(mach)
  a=_check_bad(logfile,mach,"touch %s"%file)
  SEP.spawn.run_wait(SEP.rc.remote_com(mach,"rm -f %s &"%file),400,error=None,quiet=1,ntrys=5)
  if a: return a
  file_from="%s/Spike"%SEP.paths.sepbindir
  file_to="/tmp/Spike.%s"%ext
  temp_file="%s%s.H"%(SEP.datapath.datapath(mach),ext)
  if SEP.spawn.run_wait("rcp %s  %s:%s"%(file_from,mach,file_to),400, ntrys=5,
     error=None,quiet=1):  return 1
  a=_check_bad(logfile,mach,"%s </dev/null n1=10 >%s out=stdout "%(file_to,temp_file))
  b=SEP.spawn.run_wait(SEP.rc.remote_com(mach,"rm -f  %s %s"%(file_to,temp_file)),400,error=None,ntrys=5)
  if a or b: return 1
  return None

def _check_bad(logfile,mach,command,prog=None):
  """Check to see if the return form a command makes sense"""
  err= SEP.spawn.run_wait(SEP.rc.remote_com(mach,command),
   oper_len=200.,error=None,logfile=logfile,quiet=1,ntrys=2)
  if err: 
    stat,out=commands.getstatusoutput("rm %s"%logfile)
    return err
  try: f=open(logfile)
  except: return err
  lines=f.readline()
  f.close()
  stat,out=commands.getstatusoutput("rm %s"%logfile)
  if prog:
    if re.compile("%s.+No such file"%prog).search(lines):
      return 1
  if re.compile("No such file").search(lines) or re.compile("No route").search(lines) or re.compile("Read-only").search(lines) or re.compile("Command not found").search(lines) or re.compile("Segmentation").search(lines) or re.compile("output error").search(lines): 
   return 1
  return None
 
