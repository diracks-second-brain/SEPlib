import os,sys,commands
import SEP.util
import SEP.pc_base
import SEP.distribute_prog
import SEP.paths
import SEP.spawn
import SEP.mach
import re,string
import commands
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.4.4"
                                                                                

class communicator(SEP.pc_base.communicator):
  """A class to handle communicating using mpich"""
  def __init__(self,startup=40,speed=12,mpibindir=SEP.paths.mpibindir,hostfilecom="-machinefile",includelocalhost=None,extra_args=""):
      """Initialze the mpi object
 
          startup - the maximum time it takes to start a job on the node
          speed   - the minimum speed in transfreing (in megabytes per second)
          mpibindir- directory for mpi commands

      """
      self.mpi_com=mpibindir+os.sep+"mpirun"
      SEP.pc_base.communicator.__init__(self,startup,speed,"MPI")
      self.distrib={}
      self.distrib_progs=SEP.distribute_prog.distrib_list()
      self.speed=speed
      self.startup=startup
      self.hostfilecom=hostfilecom
      if includelocalhost:
        stat,self.includelocalhost=commands.getstatusoutput("hostname")
      else: self.includelocalhost=None
      self.extra=extra_args


  def clean(self):
    """Clean up from an  MPI jobs"""
    self.distrib_progs.clean()
       
  def send_executable(self,pars,nodes):
    """Make sure that the MPI executable is on each node

      pars  - the command line arguments
      nodes - the nodes we are running on
   
      return pars with the local executable name

    """
    prog=pars[0];
    self.distrib_progs.check_add_prog(prog)
    for node in nodes: pars[0]=self.distrib_progs.prepare(prog,node)
    return pars
    
  def run_job(self,pars,nodes,logfile,nbytes):
     """Run a MPI job

         pars  - the command line arguments
         logfile  - the file to write stdout/stderr to
         nodes - the nodes to run
         nbytes- the number of bytes we are sending
     """
     nds=[] 
     nds.extend(nodes)
     pars=self.send_executable(pars,nds)
     if self.includelocalhost: nds.append(self.includelocalhost)
     mach_file,com,stat_good,distrib_file=self.local_com(pars,nodes)
     itry=0
     err=1
     err=SEP.spawn.run_wait(com,
       self.wait_time(nbytes,len(nodes)), logfile=logfile,error=None)
     sys.stdout.flush()
     err=None
#     if self.includelocalhost:
#       SEP.spawn.run_wait("rcp %s:%s /tmp"%(self.includelocalhost,stat_good),error=None)
#       SEP.spawn.run_wait("rsh -n %s  rm %s"%(self.includelocalhost,stat_good),error=None)
#     if not err:
     sys.stdout.flush()
     if not os.path.isfile(stat_good): 
       SEP.util.msg("STAT FILE DOESN'T  EXIST")
       err=1
     elif not re.compile("junk").search( commands.getstatusoutput("grep junk %s"%stat_good)[1]):  
       err=1
       SEP.util.msg("DIDN'T FINISH CORRECTLY  EXIST")
#     else:
#       SEP.util.msg("Exit status non-zero")
     sys.stdout.flush()
     if err:
       SEP.util.msg("Trouble running %s "%com)
       try: f=open(logfile)
       except: SEP.util.err("Trouble opeining up log file=%s "%logfile)
       line=f.readline()
       while line:
         SEP.util.msg(line.rstrip())
         line=f.readline()
       f.close()
       f=open(distrib_file)
       line=f.readline()
       while line:
         SEP.util.msg("PAR:%s"%line.rstrip())
         line=f.readline()
       f.close()
       
     a,b=commands.getstatusoutput("rm %s"%stat_good)
     if err: return err,logfile
#     SEP.spawn.run_wait("rm %s"%distrib_file)
#     SEP.spawn.run_wait("rm %s"%mach_file)
#     if self.includelocalhost:
#       SEP.spawn.run_wait("rsh -n %s 'rm %s'"%(self.includelocalhost,distrib_file))
     return None,logfile

  def local_com(self,pars,nodes):
    """Run the local command on pars on a series of nodes

        pars - the command line arguments
        nodes- the nodes to run on

    """
    mach=SEP.util.temp_file("mach")
    args=[self.mpi_com,"-np",str(int(len(nodes))+1),self.hostfilecom,mach]
    try: f = open (mach,"w")   #OPEN THE FILE
    except:
      SEP.util.err( "trouble opening temporary file "+mach)
    if self.includelocalhost:f.write("%s\n"%self.includelocalhost)
    for host in nodes:
      f.write(host+"\n")
    f.close()
    args.append(pars[0])
    distrib_file=self.create_distrib_file(pars[1:])
    args.append("distrib_file=%s"%distrib_file)
    stat_good=SEP.util.temp_file("goodMPI")
    args.append("stat_good=%s"%stat_good)
    args.append("verb=1")
    if self.extra: args.append(self.extra)
    com=string.join(args)
    return mach,com,stat_good,distrib_file

  def create_distrib_file(self,pars):
    """Create and return the distribution file"""
    distrib=SEP.util.temp_file("distrib")
    try: f = open (distrib,"w")   #OPEN THE FILE
    except:
      SEP.util.err( "trouble opening temporary file "+distrib)
    for par in pars:
      f.write("%s\n"%par)
    f.close()
    if self.includelocalhost: 
      err=SEP.spawn.run_wait("rcp %s %s:/tmp"%(distrib,self.includelocalhost),120)
#      err=SEP.spawn.run_wait("rm %s "%(distrib))
    return distrib
