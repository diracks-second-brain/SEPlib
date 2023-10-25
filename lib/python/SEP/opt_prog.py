import os
import SEP.util
import SEP.opt_base
import SEP.spawn
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.7.29"


                                                                                
class options(SEP.opt_base.options):
  """A simple wrapper class for running a program"""

  def __init__(self,name=None,program=None,verb=None,restart_com=""):
    """Initialize a program object

        program the path to the program
        name - the identifies for the program (defaults to the basename of prog)
        verb - whether or not to be verbose when running programs
        restart_com -Special options when restarting

    """
    SEP.opt_base.options.__init__(self,name)
    self.program=program
    if name: self.prog_name=name
    elif self.program: self.prog_name=os.path.basename(program)
    else: SEP.util.err("Must provide either program or name")
    SEP.opt_base.options.__init__(self,name)
    self.args=None
    if verb: self.prog_verb=verb
    else: self.prog_verb=None
    self.restart_com=restart_com

  def set_program(self,program):
    """Set program"""
    self.program=program

  def prep_run(self,restart=None):
    """Run a program with checking for failure

    """
    if not os.path.isfile(self.program):
      SEP.util.err("%s does not exist or is not accessible"%self.program)
    args=SEP.args.basic(name=self.prog_name)
    args.add_pars(self.return_set_params())
    beg=""
    end=""
    beg2="" 
    end2="" 
    v,e=args.par("stdin",delete=1)
    if e==1: 
      beg="<%s"%v
      beg2="stdin=%s"%v
    v,e=args.par("stdout",delete=1)
    if e==1: 
      end=">%s"%v
      end2="stdout=%s"%v

    temp_file=SEP.log.logging.new_file_name(os.path.basename(self.prog_name))
    com2="%s %s %s %s"%(beg2,self.program, args.return_set_params_string(),end2)
    com="%s %s %s %s"%(beg,self.program, args.return_set_params_string(),end)
    SEP.spawn.run_wait("echo %s >> %s/all_coms"%(com2,SEP.log.logging.return_log_dir()))
    err=SEP.spawn.run_wait(com,logfile=temp_file,verb=self.prog_verb,error=None)
    if err: 
      log=SEP.log.logging.return_last_log(os.path.basename(self.prog_name))
      SEP.util.err("%s\n%s"%(com,log))

  def get_name(self,prefix=""):
    """The name of the job"""
    return "%s%s"%(prefix,self.prog_name)

class operator(options):
  """A simple wrapper class for running a program"""

  def __init__(self,name=None,program=None,verb=None,restart_com=""):
    """Initialize a program object

        program the path to the program
        name - the identifies for the program (defaults to the basename of prog)
        verb - whether or not to be verbose when running programs
        restart_com -Special options when restarting

    """
    options.__init__(self,name,program,verb,restart_com)

  def run_in_flow(self):
    """Return whether a option is runable"""
    return None

