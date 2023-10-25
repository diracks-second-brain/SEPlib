import SEP.sep_socket
import SEP.mach
import SEP.parameter
import SEP.opt_none
import SEP.prog
import SEP.args
import os,sys,string,time,commands,signal,types,re,pwd
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.07.29"
                                                                                


def simple_section_list(nblock):
  """Create a dictionary linking section descriptors and section numbers"""
  isect_dict={}
  for i in range(int(nblock)): isect_dict[str(i)]=i
  return isect_dict
  

def simple_parameter_list(nblock):
  """Create a dictionary linking section descriptors and parameter objects"""
  sect_pars={}
  for i in range(int(nblock)):
    sect_pars[str(i)]=SEP.args.basic(name=str(i))
  return sect_pars

def decode_action(msg):
   """Decode  actions"""
   parts=msg.split(":")
   if len(parts)==4: return parts[0],parts[1],parts[2],parts[3]
   else: return "none","none","none","none"
                                                                                             
def encode_action(part1,part2,part3="none",part4="none"):
   """Encode the actions returned by the server"""
   return string.join([part1,part2,part3,part4],":")
                                                                                             
class parameters(SEP.opt_none.options):
  """A class the holds the basic parallel job parameters"""
  def __init__(self,name="PARJOB"):
    SEP.opt_none.options.__init__(self,name)
    self.add_doc_params()
  def add_doc_params(self):
    self.add_pars(_cl_par_params())


def _object_par_params():
   """Return the object parallel parameters that are usually specified by the inheritance class"""
   pgroup=SEP.args.basic(name="JUNK")
   pgroup.add_param("nmax_talk",None,default=60,doc="Maximum number of processes to use a socket")
   pgroup.add_param("files",None,doc="Dictionary of a parallel files")
   pgroup.add_param("sect_pars",None,doc="Dictionary jobid:parameters")
   pgroup.add_param("program",None,required=None,doc="Parallel program to run (required if not overriding)")
   pgroup.add_param("global_pars",None,required=None,doc="parameters that every job must have")
   pgroup.add_param("order",None,required=None,doc="Order to do jobs (semi-random)")
   return pgroup


def _cl_par_params():
   """Return the basic parallel parameters that are usually specified on the commandline"""
   pgroup=SEP.args.basic(name="JUNK")
   pgroup.add_doc_param("nmax_talk",default=60,doc="Maximum number of processes to use a socket")
   pgroup.add_doc_param("device",default="eth1",doc="Device connected to the cluster")
   pgroup.add_doc_param("port",default=50000,doc="First port to look for an open connection")
   pgroup.add_doc_param("redo",default="y",doc="Whether or not to redo a job on a dead node")
   pgroup.add_doc_param("redo_collect",default="n",doc="Whether or not to redo a job if a node dies during collect")
   pgroup.add_doc_param("status_interval",default=10.,doc="sleep time in job loop (between checking message files)")
   pgroup.add_doc_param("alive_interval",default=600.,doc="how often to check if the nodes are working correctling")
   pgroup.add_doc_param("pverb",default=0,doc="Verbosity level for parallel operatorions",required=None)
   pgroup.add_doc_param("mpich_includelocalhost",None,doc="Include local host name in the machinefile  ")
   pgroup.add_doc_param("mpich_extra_args",None,doc="Extra arguments for mpi commands")
   pgroup.add_doc_param("mpich_hostfilecom","-machinefile",doc="How to designate a machinefile to mpirun ")
   pgroup.add_doc_param("mpich_startup",30.,doc="When using mpich, max time to start a process on a node (in seconds)")
   pgroup.add_doc_param("mpich_speed",10.,doc="When using mpich, transfer speed (in seconds)")
   pgroup.add_param("mpich_bindir",None,doc="When using mpich, binary directory defaults to the one used in configuring SEPlib",required=None)
   pgroup.add_doc_param("mach_mfile","mfile",doc="When obtaining machines from machinefile, the name of the machinefile")
   pgroup.add_doc_param("remote_shell","rsh -n",doc="the shell command to use when starting remote jobs")
   pgroup.add_doc_param("mach_njobs",2000,doc="The maximum number of jobs to run at one time")
   pgroup.add_param("attempts",None,default=3,doc="The number of attempts to run a job")
   pgroup.add_param("logdir","stat",default="stat",doc="Directory for logdir")
   return pgroup 


