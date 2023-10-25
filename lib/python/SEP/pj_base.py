import SEP.stat_sep
import SEP.args
import SEP.mach_file
import SEP.mach
import SEP.datapath
import SEP.pc
import SEP.pc_mpich
import SEP.distribute_prog
import SEP.opt_base
import SEP.prog
import SEP.pj_func
import SEP.rc
import SEP.par_msgs
import os,sys,string,time,commands,signal,types,re,pwd
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2006.1.10"
                                                                                


class parjob(SEP.stat_sep.status):
  """A parallel object"""

  def __init__(self, name="PAR",  **kw):
    """Initialize a parallel job object

       Required parameters:
       files -  a dictionary of a parallel files
       sect_pars   - the parameters associated with the different jobs
       program     - the  program to run in parallel 
 
       Optional parameters:
         
       nmax_talk - the maximum number of processes for each sect (60)
       port      - the first portion to look for an open connection (50000)
       redo      - whether or not to redo jobs that finished on node if the node dies
       status_interval- sleep time in job loop (between checking message files) (.23)
       alive_interval-how often to check if the nodes are working correctling (600)
       verb        - how verbose to be, default 0
       shell       - the shell command to use ('rsh -n')  
       restart_com - the command to use if we are restarting a job ("")
       njobs       - the maximum number of jobs to un at one time (2000)
       attempts    - the number of attempts to run a job (3)
       global_pars - parameters that every job must have (defaults to none)
       logdir      - directory for loggin

    """
   

    #save the init parameters
    self.kw=kw
    print "CHECK NAME",name
    self.name=name
    self._init_check()

  def _init_check(self):
    """Check to make sure parameters make sense"""

    self.fake=None

    #the list of parallel files
    self.files=self._pj_option("files",err=1)

    #verbosity level
    self.verb=self._pj_option("pverb",0)
    if not self.verb: self.verb=0
    if str(self.verb)[0]=='n' or str(self.verb)[0]=='N': self.verb=0
    if str(self.verb)[0]=='y' or str(self.verb)[0]=='y': self.verb=2
    self.verb=int(self.verb)



    self.status_interval=float(self._pj_option("status_interval",10.))
    self.alive_interval=float(self._pj_option("alive_interval",900.))


#    #put all of the job commands to a file
#    self.all_coms="%s/%s"%(SEP.log.logging.return_log_dir(),"all_coms")

    #parameters for the various jobs
    self.sect_pars=self._pj_option("sect_pars",err=1)


    #create the job list
    job_list=self.sect_pars.keys()
    order=self._pj_option("order",job_list)
    #add in the collection
    for file in self.files.keys(): 
      job_list.append("collect-%s"%file)
      order.append("collect-%s"%file)
    self.notcollect=re.compile("collect-")


    #create the status object
    SEP.stat_sep.status.__init__(self,"job.stat",
      job_list, njob_extra=4,order=self._pj_option("order",job_list))




    #add to command to restart the job;
    self.restart_com=self._pj_option("restart_com","")


    #list of programs to be distributed
    self.prog_list=SEP.distribute_prog.distrib_list()
    

    #program to run ( simple parallelization)
    self.program=self._pj_option("program")
    
    

    #number of attempts to start a job
    self.attempts=self._pj_option("attempts",3)

    #global parameters (all jobs need these pars)
    self.global_pars=self._pj_option("global_pars",SEP.args.basic(name="par_job"))

    self.setup_mach_rsh()
 
    #maximum number of jobs
    self.njobs=int(self._pj_option("njobs",2000))

    self.msg_objs=[] #object to receive messages from
    self.socket_objs=[] #socket objects

    #translation from machines to objects
    self.mlabel_talk={}

    #maximum number of processes that should use on socket
    self.nmax_talk=int(self._pj_option("nmax_talk",60))

    self.msg_objs.append(SEP.par_msgs.msg_obj("%s/msgs.%d"%(SEP.log.logging.return_log_dir(),0))) 
    self.first_port=int(self._pj_option("port",50000))
    self.next_port=self.first_port
    for i in range(self.njobs/self.nmax_talk+1):
      self.msg_objs.append(SEP.par_msgs.server_msg_obj(
        "%s/msgs.%d"%(SEP.log.logging.return_log_dir(),i+1),
         self.nmax_talk,self._pj_option("device","eth1")))


  def setup_mach_rsh(self):
    """Setup machine"""
    #machine object (later expand on the concept)
    if not SEP.mach.mach: #if not already specified
      SEP.mach.mach=SEP.mach_file.machine(self._pj_option("mach_mfile"),
         self._pj_option("mach_nmax"))
    #shell to run to access the nodes
    SEP.rc.shell=self._pj_option("remote_shell","rsh -n ")
    if not SEP.rc.shell: SEP.rc.shell="rsh -n"
    

    self.mach=SEP.mach.mach

  def setup_communicator(self):
    """Setup the communicator for the nodes, defaults to mpich"""
    if not SEP.pc.return_communicator():
      startup=self._pj_option("mpich_startup",10.)
      hostfilecom=self._pj_option("mpich_hostfilecom","-machinefile")
      extra_args=self._pj_option("mpich_extra_args",None)
      includelocalhost=self._pj_option("mpich_includelocalhost",None)
      speed=self._pj_option("mpich_speed",10.)
      mpibindir=self._pj_option("mbindir",SEP.paths.mpibindir)
      mpich=SEP.pc_mpich.communicator(startup,speed,mpibindir,hostfilecom,includelocalhost,extra_args)
      SEP.pc.set_communicator(mpich)

  def _check_files(self,par_list):
    """ Check to make sure parallel files make sense

       Given a list of parallel files make sure that all of the files
       have the same number of sections (if not of type COPY)

    """
    nsect=len(par_list)
    self.multi_start=1
    for key,file in self.files.items():
      if file.type != "COPY":
        if nsect!=file.nsect():
          err = "Number of sections  %d not equal to number"%nsect
          err+= " of parts %s in file=%s" % (str(file.nsect()),file.file_name)
          SEP.util.err(err)
      elif file.usage[0:3]=="OUT": self.multi_start=None

  def collect_restart(self,key):
    stat=self.get_key_status_elem(key,4)
    ifinish=0
    if stat and stat != "none": 
      finishS=re.compile("finished=(\d+)")
      if os.path.isfile(stat):
        f=open(stat)
        lines=f.readlines()
        for line in lines: 
          res=finishS.search(line)
          if res: ifinish=res.group(1)
    if ifinish >0 : return ["restart=y","finished=%s"%str(ifinish)]
    return []

  def _collect(self):
     """Collect the data for all nodes where a process isn't currently running"""
     key_list=[]
     self._lock_all()
     mach_dict=self.get_other_dict(self.remove_collect_jobs(self.get_by_status("finished")))
     running=self.mach.get_by_status("running")
     running.extend(self.mach.get_by_status("sent"))
     running=self.mach.machs_from_labels(running)
     for key,val in mach_dict.items():
       if running.count(val) ==0 : 
         key_list.append(key)
     if len(key_list)>0:
       for tag,file in self.files.items(): 
         if tag==">": taguse="stdout"
         elif tag=="<": taguse="stdin"
         else: taguse=tag
         lab="collect-%s"%tag
         lab2="collect-%s"%taguse
         stat=self.get_key_status_elem(lab,0)
         if stat!="finished":
           extra=self.collect_restart(lab)
#           if tag=="outtag": SEP.util.err("%s"%string.join(extra))
           log=SEP.log.logging.new_file_name(lab2);
           self.set_key_status_elem(lab,0,"running")
           self.set_key_status_elem(lab,4,log)
           self._unlock_all()
           self._lock_all()
           if file.collect(key_list,log,extra):
             redo=str(self._pj_option("redo_collect","n"))
             if redo[0]=="n" or redo[0]=="N" or redo[0]=='0': redo=None
             if redo:
               SEP.util.warn("Trouble collecting %s , redoing "%file.file_name)
               extra=self.collect_restart()
               self.set_key_status_elem(lab,0,"running")
               self.set_key_status_elem(lab,4,log)
               self._unlock_all()
               if self.verb >1:  SEP.util.msg("Checking node health")
               self.check_nodes()
               self._job_server()
               self._lock_all()
               if file.collect(key_list,[" "]):
                 SEP.util.err("Trouble collecting %s  "%file.file_name)
             else: SEP.util.err("Trouble collecting %s  "%file.file_name)
           self.set_key_status_elem(lab,0,"finished")
           self._unlock_all()
           self._lock_all()
     self._unlock_all()

  def _lock_all(self,read=1 ):
    """Lock all status files
       
       Make sure this is the only process that can update the status

    """
    
    self.mach._lock_it()
    if not self.fake:
      self._lock_it(read)
      for name,file in self.files.items(): file._lock_it(read)

  def _unlock_all(self,write=1 ):
    """Unlock all status files"""
    if not self.fake:
      self._unlock_it(write)
      for file in self.files.values(): file._unlock_it(write)
    self.mach._unlock_it()


  def update_parfile_dict(self,tag,parfile,props=None):
    """Modify the parfile dictionary

       When doing inversion the name of the model and datafile changes
       This routine allows for the change

    """
    self.files[tag]=parfile
    if props:
      for k,v in props.items(): self.files[tag].kw[k]=v

  def set_global_par_dict(self,par):
    """Set the global database paramter list """
    self.global_pars=SEP.args.basic(name="name")
    self.global_pars.add_pars(par)

  def return_global_par_dict(self):
    aa=SEP.args.basic(name="name")
    aa.add_pars(self.global_pars)
    return aa
    

  def set_global_par(self,par,value):
    """Set a parameter value to be used on all jobs"""
    self.global_pars.param(par,value)

  def _init_pids(self):
    """Mark that we are begining a new job"""
    self.key_pids={}
    self.pids=[]


  def _new(self):
    """Signify that this a new parallel job"""
    self._init_pids()
    self._lock_all(read=None)
    self.init_all()
    for msg in self.msg_objs: msg.init()
    for file in self.files.values(): 
      file.init_file()
    self._unlock_all()
      

  def clean_files(self):
    """Clean the distributed executables and the distributed file parts"""
    SEP.pc.clean()
    self.prog_list.clean()
    for tag,file in self.files.items(): 
      if self.verb >1:  SEP.util.msg("Cleaning up %s"%file.file_name)
      file.clean_parts()
    
  def clean(self):
     """Clean up all files except output file(s)"""
     for msg in self.msg_objs:  msg.clean();
     self.clean_files()

  def restart(self):
    """Restart a job"""
    self._init_pids()
    self._logfile_update()
    self._msg_update()
    restart=SEP.stat_sep.status.restart(self,1)
    for file in self.files.values(): file.restart(restart=1)

  def _logfile_update(self):
    """Update the message files based on  """
    self._lock_all()
    todo_list=self.get_elem_val(0,"running")
    some_progress=self.elem_not_list(["none"],4,keys=todo_list)
    log_files=self.get_stat_elem(4,some_progress)
    machs=self.get_stat_elem(1,some_progress)
    myre=re.compile("finished=(\d+)")
    line_list,err=self.msg_objs[0].read_delete()
    for key,file in log_files.items():
      mach=machs[key]
      if  file and file!="none":
        try: 
          f=open(file)
          lines=f.readlines()
          for line in lines:
            a=myre.search(line)
            if a:
              msg=SEP.pj_func.encode_action("progress",key,mach,"finished=%s of"%str(a.group(1)))
              line_list.append("%s\n"%msg)
          f.close()
        except:  SEP.util.msg("%s doesn't exist ... skipping"%file)
      self.msg_objs[0].write_msgs(line_list)
    self._unlock_all()
       

  def check_nodes(self,type="free"):
    """ Check the status of a series of nodes given by their machine label"""
    check=[]
    self._lock_all()
    machlabel=self.mach.get_by_status(type)
    for label in machlabel: 
      mach=self.mach.mach_from_label(label)
      if check.count(mach)==0:
        self.check_node_status(label)
        check.append(mach)
    self._unlock_all()


  def _init_run(self,restart):
    """Initialize a run. If restarting set restart=1 """
    tt=self.files.keys()[0]
    self.died_on_node={}
    if not restart: self._new()
    self.mach.init_all()
    if self.verb >1:  SEP.util.msg("Checking node health")
    self.check_nodes()
    self.failures=[]
    self.key_pids={}
    self.not_first=None
    self.key_finish={}
    for msg in self.msg_objs[1:]: msg.started=None
    for file in self.files.values():file.verb=self.verb
    if restart: self.restart()
    self.setup_communicator()
    self._check_files(self.sect_pars)

  def run_job(self,restart=None,pars=None):
    """ Start a parallel job (restart=1 if restarting) """
    self._init_run(restart)
    self.run_it()


  def run_it(self):
    stat=self.start_jobs()
    if stat== None: SEP.util.err("No jobs to run")
    self._lock_all()
    self.update_nrunning() 
    self._unlock_all()
    if stat=="Full":
      SEP.util.msg("All nodes are full. Will retry in 5 seconds")
      while "Full"==stat: time.sleep(5.)
      SEP.util.msg("Starting jobs")
    elif stat == "Finished" and self.nrunning<=0:  self._collect()
    else:
      self._job_server()
      self._collect()

  def _msg_update(self):
   """Update the status using the message files"""
   self._lock_all()
   for i in range(len(self.msg_objs)-1,-1,-1):
     msg=self.msg_objs[i]
     if msg.started:
       lines,err=msg.read_delete()
       if err: 
         self.exit_clean(-1)
       for line in lines:
         oper,jobid,machid,extra=SEP.pj_func.decode_action(line.rstrip("\n"))
         if   oper== 'record':  self._record(jobid,machid,extra)
         elif oper== 'finish':  self._finish(jobid,machid,extra)
         elif oper== 'progress':  self._progress(jobid,machid,extra)
         elif oper== 'error':  self._error(jobid,machid,extra)
         elif oper== 'thread':   self._slave_done(jobid,machid,extra)
         else: SEP.util.err("unknown type:%s:\n%s%s"%(oper,line,lines))
   self._unlock_all()
   

  def _del_socket_use(self,machid):
    """Mark that a given machine label is no longer using a socket"""
    #when restarting we want to do nothing so the if conditional
    if self.mlabel_talk.has_key(machid):
      self.msg_objs[self.mlabel_talk[machid]].remove_use(machid)
      del self.mlabel_talk[machid]
   

  def _add_socket_use(self,machid):
    """Find/create a socket for a given machine id to use"""
    for i in range(1,len(self.msg_objs)):
      good,pid,next_port=self.msg_objs[i].add_use(machid,self.next_port)
      if good:
        self.mlabel_talk[machid]=i
        if pid:
          self.pids.append(pid)
          self.next_port=next_port
        return i
    SEP.util.error("Ran out of msg_objects increase njobs and nmax_talk")

  def _slave_done(self,jobid,machid,extra):
    """Mark that a slave process has finished"""
    self._del_socket_use(machid)
    #check to make sure we haven't started an extra job
#    good=1
#    list=self.get_elem_val(1,machid)
#    if list:
#      for i in list:
#        if i != jobid: good=0
#    if list:
#      SEP.util.msg("job=%s mach=%s good=%s list=%s"%(jobid,machid,good,string.join(list)))
#    else: 
#      SEP.util.msg("job=%s mach=%s good=%s list=%s"%(jobid,machid,good,"None"))
#    if good==1: self.mach.update_status(machid,["free"])
    #only make a node free if it thinks it is running this job
    id=self.mach.get_key_status_elem(machid,1)
    if id == jobid: self.mach.update_status(machid,["free"])
    self.key_finish[jobid]=time.time()

  def _check_slave_status(self):
    """Check to see if a slave job  exited  incorrectly"""
    for key,myt in self.key_finish.items():
       if time.time() > myt+300: 
         stat=self.get_key_status_elem(key,0)
         mach=self.get_key_status_elem(key,1)
         extra=self.get_key_status_elem(key,2)
         if stat != "finished" and stat!="collected" and mach !="none":
           if extra != "seperr": extra="died"
           self._add_failure(key,mach,extra)
           os.waitpid(self.key_pids[key],0)
           del self.key_pids[key]
         del self.key_finish[key]

  def _check_status(self):
    """Check whether or not we have reached the criteria for quitting/finishing"""
    self._lock_all()
    self._check_slave_status()
    if self.get_by_status("fail"): self.exit_clean(-1)
    self.update_nrunning()
    self._unlock_all()

  def _record(self,jobid,machid,extra):
     """" Record that a task is running"""
     old_stat=self.get_key_status_elem(jobid,0)
                                                                                

     if old_stat=="sent":
       if self.verb >2: SEP.util.msg("  Job %s on %s registerd"%(jobid,machid))
       self.mach.update_status(machid,["running",jobid,extra]) 
       self.update_status(jobid,["running",machid])
       for file in self.files.values(): file.record(jobid)
       if self.died_on_node.has_key(jobid): del self.died_on_node[jobid]


  def _progress(self,jobid,machid,extra):
    """Register progress in a job"""
    old_stat=self.get_key_status_elem(jobid,0)
    old_extra=self.get_key_status_elem(jobid,2)
    src=re.compile("finished=(\d+)\s+")
    old=src.search(old_extra)
    new=src.search(extra)
    if old_stat!="finished" and old_stat!="collected" and not self.died_on_node.has_key(jobid):
      if not new or not old or int(old.group(1))< int(new.group(1)):
        if self.verb >1:
          SEP.util.msg(" %s- Job %s on %s progress=%s"%(self.name,jobid,machid,extra))
        self.update_status(jobid,["running",machid,extra])

  def _finish(self,jobid,machid,extra):
     """ Mark that a job as finished"""
     if not self.died_on_node.has_key(jobid):
       for file in self.files.values(): file.done(jobid)
       if self.verb >1:
          SEP.util.msg("Job %s- %s on %s finished"%(self.name,jobid,machid))
       self.update_status(jobid,["finished",machid])
       if self.key_finish.has_key(jobid): del self.key_finish[jobid]
     
  def prep_job(self,key,mlabel):
    """Do any prep work needed for job key to be run on mach label mlabel"""
    return None
    prog,mach=self.local_prog_mach(key,mlabel)
    if prog:
      return SEP.spawn.run_wait("rcp %s %s:%s"%(self.program,mach,
       prog),200,ntrys=5)
    return None
     
  def clean_job(self,key,mlabel):
    """Clean up after job (key) finished successfully"""
    return None
    prog,mach=self.local_prog_mach(key,mlabel)
     
  def _add_failure(self,key,mach,extra):
     """Error when running job"""
     self.set_failure(key,self.get_failures(key)+1)
     count=self.get_failures(key)
     self.failures.append(self._fail_label(key,mach))
     if count==self.attempts: 
       SEP.util.msg("To many failures for %s, dying"%key)
       self.exit_clean(-1)
     else: self.update_status(key,["todo"])
     #Future mark machine as failing????
     SEP.util.msg("Failure: key=%s mach=%s error=%s"%(key,mach,extra))
     self.check_node_status(mach)


  def check_node_status(self,mlabel=None,mach=None):
     """Check to see if a node is dead"""
     if mlabel: 
       mach=self.mach.mach_from_label(mlabel)
     elif not mach: SEP.util.err("Must provide mach or mlabel")
     stat=SEP.mach_base.check_alive(mach)
     if stat:
       SEP.util.msg("%s isn't functional (dead, mounting problem???)"%mach)
       labels=self.mach.label_list(mach)
       keys_on_node=[]
       key1=[]
       for m in labels:
         self.mach.set_key_status_elem(m,0,"dead")
         if not self.fake: key1.extend(self.get_elem_val(1,m))
       #only deal with keys that aren't running (others will happen eventually)
       if not self.fake:
         for  key in key1:
           stat=self.get_key_status_elem(key,0)
           if stat!="running" and stat!="sent": keys_on_node.append(key)
         self.handle_dead_jobs(keys_on_node)


  def handle_dead_jobs(self,labels):
    """Handle dead jobs on  a series of machine labels"""
    redo=str(self._pj_option("redo","y"))
    if redo[0]=="n" or redo[0]=="N" or redo[0]=='0': redo=None
    for key in  labels:
      stat=self.get_key_status_elem(key,0)
      if stat!="collected":
        if stat!="finished" or redo: 
          self.died_on_node[key]=1
          self.set_key_status_elem(key,0,"todo")
          self.update_status(key,["todo","none","none","none"])
          for file in self.files.values(): file.redo(key)

     
  def update_nrunning(self):
    """Calculate how many jobs are currently running"""
    self.nrunning=len(self.sect_pars)-len(self.get_by_status("finished"))- len(self.get_by_status("collected"))

  def _error(self,id,machid,extra):
     """Error when running job"""
     self.update_status(id,["running",machid,"seperr"])
     return 1

  def _fail_label(self,key,mlabel):
    """return label for failures"""
    return "%s:%s"%(key,self.mach.mach_from_label(mlabel))

  def command_func(self,key,mlabel,par_args,job_args,tags):
    """Return the command to run given the job descriptor, job_string and file tags

       key      - the jobid 
       mlabel   - the machine label we will be running on
       par_args - the parameters for the parallel jobs
       job_args - the parameters for job args
       tags     - the local file tags

       This should be overwritten when doing more complex jobs

    """

    par_args.add_pars(job_args)
    if not self.program: 
      SEP.util.err("Must override command_func when program not provided")
    prog=self.prog_list.prepare(self.program,self.mach.mach_from_label(mlabel))
    run ="%s sep.begin_par=1 sep.end_par=1 %s %s"%(prog,
      par_args.return_set_params_string(), string.join(tags))
    return run

  def return_par_files(self):
    """Return the parallel files"""
    return self.files 

  def set_failure(self,key,val):
     """Set the number of failures for a given job"""
     self.set_key_status_elem(key,3,val)
                                                                                
                                                                                
  def status_vals(self,keys):
     """Return the status of a series of keys"""
     return self.get_stat_elem(0,keys)

  def get_other_dict(self,key_list):
     """Get the other status given a key list"""
     return self.get_stat_elem(1,key_list)
                                                                                
  def get_failures(self,key):
     """Get the the number of failures of a given job"""
     v= self.get_key_status_elem(key,3)
     if not v or v == "none": v=0
     else: v=int(v)
     return v

  def start_jobs(self):
    """ Start a series of jobs """
    commands,err=self.build_commands()
    start_dt=1.5
    if self.not_first: isleep=-start_dt
    else:              
      isleep=2-start_dt
      self.not_first=1 
    if err == "Run":
      for key,cm in commands.items(): 
         log=SEP.log.logging.old_file_name("%s.%s"%(self.name,key))
#         SEP.spawn.run_wait("echo %s >>%s"%(cm,self.all_coms),40)
         mlabel=self.get_key_status_elem(key,1)
#         SEP.util.err(cm)
         pid=os.fork()
         isleep=isleep+start_dt
         if mlabel == "none": SEP.util.err("Trouble getting key label %s"%key)
         if not pid: #child process starts jobs
           time.sleep(isleep)
           a= self.prep_job(key,mlabel)
           if a:
             self.msg_objs[0].action("error:%s:%s:prep failure"%(key,mlabel))
           SEP.spawn.run_wait(cm,error=None,logfile=log)
           self.msg_objs[0].action("thread:%s:%s:none"%(key,mlabel))
           self.clean_job(key,mlabel)
           sys.exit(0)
         else: 
           self.key_pids[key]=pid
    return err



  def _job_server(self):
     """Update node status and try to start more jobs"""
     sleep_time=self.status_interval
     next_time=time.time()+self.alive_interval
     sent_list=[]
     while 1:
       time.sleep(sleep_time)
       self._check_status()
       self._msg_update()
       stat=self.start_jobs()
       if self.nrunning<=0: 
         self._lock_all()
         self.exit_clean()
         self._unlock_all()
         return 
       if time.time()>next_time:
         if self.verb >1:  SEP.util.msg("Checking node health")
         self.check_nodes("running")
         self._lock_all()
         next_time=time.time()+self.alive_interval
         dlist=self.mach.get_by_status("dead")
         if dlist:
           mlist=[]
           for mach in dlist: mlist.append(self.mach.mach_from_label(mach))
           mlist=SEP.util.condense(mlist)
           for mach in mlist:
             stat=SEP.mach_base.check_alive(mach)
             if not stat:
               SEP.util.msg("%s now seems functional"%mach)
               labels=self.mach.label_list(mach)
               for m in labels: self.mach.set_key_status_elem(m,0,"free")
         self._unlock_all()

  def jobs_to_do(self):
    """Return the list of jobs that haven't completed""" 
    return self.remove_collect_jobs(self.get_by_status("todo"))

  def remove_collect_jobs(self,list):
    """Parse out the collect jobs"""
    list_out=[]
    for key in list: 
      if not self.notcollect.search(key): list_out.append(key)
    return list_out
    
         
  def assign_jobs(self):
     """Return the list of jobs to assign to each node """
     todo_list=self.jobs_to_do()
     if self.multi_start:
       machlabel=self.mach.free_labels()  #get list of machines we can run on
     else:
       machlabel=self.mach.free_mach()  #get list of machines we can run on
     mach_send={}
     for mlabel in machlabel:
       assign=0
       i=0
       while assign ==0 and  i < len(todo_list):
          flabel=self._fail_label(todo_list[i],mlabel)
          if self.failures.count(flabel)==0:
            assign=1 
            mach_send[todo_list[i]]=mlabel
            del todo_list[i]
          else: i=i+1
     return mach_send



  def server_pars(self,mach,job_pars):
    """Return the socket parameters need for a job"""
    icom=self._add_socket_use(mach)
    return self.msg_objs[icom].server_pars(job_pars)

           

  def build_commands(self):
    """Build commands to run on free nodes"""

    self._lock_all()
    mach_send=self.assign_jobs()
    if not mach_send: 
      if self.jobs_to_do():
        self._unlock_all()
        return None,None
      if self.get_by_status("running"): 
        self._unlock_all()
        return None,None
      self._unlock_all()
      return None,"Finished"

     
    #setup all of the parralel files for the job
    tag_job={}
    for key in mach_send.keys(): tag_job[key]=" ";

    for tag,file in self.files.items():
      if tag==">": taguse="stdout"
      elif tag=="<": taguse="stdin"
      else: taguse=tag
      log=SEP.log.logging.new_file_name("send.%s"%taguse)
      err,tags=file.tags(mach_send,log)  #tupple  label:tag
      if err: 
        for mach in mach_send.values(): self.check_node_status(mach)
        self._unlock_all()
        return None,None
      for key,tag in  tags.items(): tag_job[key]=tag_job[key]+" "+tag

    #time to build the commands
    run_this={}
    for key,mach in mach_send.items():
      m=self.mach.mach_from_label(mach)
      self.update_status(key,["sent",mach_send[key]])#update our status
      self.mach.update_status(mach,["sent",key,str(time.time())])  #update machine status
      job_pars=SEP.args.basic(seppar=self.global_pars,name=key)
      job_pars=self.server_pars(mach,job_pars)
      job_pars.add_param("sep.jobid",key)
      job_pars.add_param("datapath",SEP.datapath.datapath(m))
      job_pars.add_param("sep.mach_label",mach_send[key])
      job_args=SEP.args.basic(seppar=self.sect_pars[key],name="sect_par")
      self.set_key_status_elem(key,4,SEP.log.logging.new_file_name("%s.%s"%(self.name,key)))
      stat=self.get_key_status_elem(key,2)
      srch=re.compile("(.+)=(.+)")
      if self.restart_com: g=srch.search(self.restart_com)
      else:g=None
      if g: 
        if job_pars.param(g.group(1)):
          job_pars.del_par(g.group(1))
      if stat and stat != "none":
        job_args.add_string(stat)
        if self.restart_com:
          job_args.add_string(self.restart_com)
      if self.verb >0:
        if self.verb ==1 or self.verb==2:
          SEP.util.msg("Sending job %s to %s "%(key,mach))
        elif self.verb ==3:
                SEP.util.msg("Sending job %s to %s pars=[%s]"%(key,mach,self.sect_pars[key].return_set_params_string()))
        else:
          SEP.util.msg("Sending job %s to %s pars=[%s]"%(key,mach,job_pars.return_set_params_string()))
      run=self.command_func(key,mach,job_pars,job_args,tag_job[key].split())
      run_this[key]=SEP.rc.remote_com(m, run)
    self._unlock_all()
    return run_this,"Run"

  def exit_clean(self,stat=None):
    """Kill (or exit) all slave threads"""

    #first terminate all of the jobs
    for key,pid in self.key_pids.items():
      if stat: os.kill(pid,9)
#      else: os.waitpid(pid,0)


    while 1:
       pid, sts = os.waitpid(-1, os.WNOHANG)
       if pid == 0: break
    #exit all the socket servers
    self.exit_servers()


#    #now make sure that all of the sockets are dead
#    for pid in self.pids: 
#      os.waitpid(pid,0)

    if stat: sys.exit(stat)

  def exit_servers(self):
    """Exit all of the socket servers"""

    ip=self.msg_objs[1].ip_server()
    port_nums=[]
    for i in range(1,len(self.msg_objs)):
      if self.msg_objs[i].started: 
        port_nums.append(str(self.msg_objs[i].port_number()))

    while 1:
      mach=self.mach.not_dead()
      prog="%sSend_msg.%s.%s"%(SEP.datapath.datapath(mach),
         pwd.getpwuid(os.getuid())[0],os.getpid())
      err=SEP.spawn.run_wait("rcp %s/Send_msg %s:%s"%(SEP.paths.sepbindir,
        mach,prog),200 ,quiet=1,ntrys=5)
      if err: self.check_node_status(mach=mach)
      
      else:
        err1=SEP.spawn.run_wait(SEP.rc.remote_com(mach,'%s ip="%s" port=%s msg_type=%s %s'%(prog,ip,string.join(port_nums,","),"exit",
         "</dev/null >/dev/null")),30, error=0,ntrys=3)
        err2=SEP.spawn.run_wait(SEP.rc.remote_com(mach,"rm -f %s"%(prog)),
          30,error=0,quiet=1,ntrys=3)
        if not err1 and not err2: return

  def _pj_option(self,par,default=None,err=None):
    """Simple parameter checking

       par     - parameter to check for 
       default - default parameter value
       err     - whether or not to give an error if the parameter doesn't exist

    """
    if self.kw.has_key(par): 
      return self.kw[par]
    elif default: return default
    elif err: SEP.util.err("Missing required parameter %s "%par)
      


class par_job(parjob,SEP.opt_base.options):
  def __init__(self,name="PAR"):
    """Initialize a parallel object """
    self.verb=0
    self.fake=1
    self.name=name
    SEP.opt_base.options.__init__(self,name)
    self.add_doc_params()

  def add_doc_params(self):
     """Add the required files for a paralllel job"""
     self.add_pars(SEP.pj_func._cl_par_params())
     self.add_pars(SEP.pj_func._object_par_params())


  def prep_run(self,restart=None):
     """Check to make sure the parameters are reasonable/add additional parameters"""
     self.prep(restart)
     self.run(restart)

  def prep(self,restart=None):
     kw={}
     parjob.__init__(self,self.name,**kw)

  def run(self,restart=None):
     self.run_job(restart)



  def delete_cl_par_params(self):
    """Delete the command line parameters of the parallel job"""
    for i in SEP.pj_func._cl_par_params().return_pars(): 
      self.del_par(i)

  def delete_object_par_params(self):
    """Delete the object parameters of the parallel job"""
    for i in SEP.pj_func._object_par_params().return_pars(): self.del_par(i,None)


  def set_cl_par_params(self,pars):
    """Set the commandline par parameters from an SEP.par"""
#    self.add_pars(SEP.pj_func._cl_par_params(),None)
    self.add_pars(pars,None)
#    for i in SEP.pj_func._cl_par_params().return_pars(): 
#      self.add_param(i,pars.param(i))
    
  def _pj_option(self,par,default=None,err=None):
    """Simple parameter checking

       par     - parameter to check for 
       default - default parameter value
       err     - whether or not to give an error if the parameter doesn't exist

    """
    return self.param(par,default=default,error=err)
      

     

