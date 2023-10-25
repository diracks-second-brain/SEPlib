import SEP.util
import SEP.vec_base
import SEP.prog
import time,os,string,types,sys
import SEP.op_oc
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.11.29"
                                                                                
class operator(SEP.opt_prog.operator,SEP.op_oc.operator):
  """An operator class for simple out of core operations"""


  def __init__(self,prog,name=None,verb=None,msg=None):
    """Initialize a simple out of core operator 
 
       prog    - the program to run
       name    - the name associated with the operator (defaults to prog)
       verb    - whether or not to be verbose when running
       msg     - the message to print out when executing

    """
    if not name: name=prog
    SEP.op_oc.operator.__init__(self,name,verb,msg)
    SEP.opt_prog.operator.__init__(self,name)
    self.prog=prog
    self.add_doc_params()


  def add_doc_params(self):
    self.run_pars=SEP.args.basic(name="runpars")
    self.run_pars.add_doc_param("restart_com"," ",doc="Command line option when restarting",export=None)
    self.run_pars.add_doc_param("adj_com","adj=y ",doc="Command line option when running adjoint",export=None)
    self.run_pars.add_doc_param("add_com","add=y",doc="Command line option when adding",export=None)
    self.run_pars.add_doc_param("no_add_com","add=n",doc="Command line option when not adding",export=None)
    self.run_pars.add_doc_param("domain_tag","model=",doc="Command line option for input",export=None)
    self.run_pars.add_doc_param("range_tag","data=",doc="Command line option for output",export=None)
    self.run_pars.add_doc_param("domain",doc="Command line option when adding",export=None)
    self.run_pars.add_doc_param("range",doc="Command line option when adding",export=None)
    self.add_pars(self.run_pars)

  def del_doc_params(self):
    """Delete the parameters associated with setting up an out of core operator"""
    for key in self.run_pars.keys(): self.del_par(key)

  def prep_run(self,restart=None):
    """Check to make sure parameters make sense before running"""
    self.set_operator(self.param("domain"),self.param("range"))


  def build_basic_com(self,domain,range):
    """Return the basic command to run (e.g. without adj, add, and restart")"""
    return "%s %s%s %s%s %s"%(self.prog,self.param("domain_tag"),domain.file_name,
     self.param("range_tag"),range.file_name,self.return_set_params_string())

  def forward_pars(self,domain,range,add,restart):
    """Build the basic command we are going to run when running forward"""
    basic=self.build_basic_com(domain,range)
    if add    : basic=basic+" %s"%self.param("add_com")
    else    : basic=basic+" %s"%self.param("no_add_com")
    if restart: basic=basic+" %s"%self.param("restart_com")
    return basic

  def adjoint_pars(self,domain,range,add,restart):
    """Build the basic command we are going to running the adjoint"""
    return "%s %s"%(self.forward_pars(domain,range,add,restart),self.param("adj_com"))
    


  def forward_op(self,model,data,add,restart):
    """Run the forward operation
 
        model  - the model vector
        data   - the data  vector
        add    - whether or not we are adding
        restart- whether or not we are restarting

    """
    self.run_com(self.forward_pars(model,data,add,restart))


  def adjoint_op(self,model,data,add,restart):
    """Run the adjoint operation
 
        model  - the model vector
        data   - the data  vector
        add    - whether or not we are adding
        restart- whether or not we are restarting

    """
    self.run_com(self.adjoint_pars(model,data,add,restart))

  def run_com(self,com):
    """Run a command"""

    tfile=None
    if self.verb: tfile=SEP.util.temp_file(os.path.basename(self.prog))

    SEP.spawn.run_wait(com,logfile=SEP.log.logging.new_file_name(self.op_name()))
