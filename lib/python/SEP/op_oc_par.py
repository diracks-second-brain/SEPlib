import time,os,string,types,sys
import SEP.util
import SEP.args
import SEP.vec_base
import op_oc
import SEP.pj_base
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.4.4"

class operator(op_oc.operator,SEP.pj_base.par_job):

  def __init__(self,name,verb=None,msg=None):
    """An operator class for simple coarse grain parallelization operators
                                                                                                                   
       name    - the name associated with the operator 
       verb    - whether or not to be verbose when running
       msg     - the message to print out when executing
                                                                                                                   
    """
    SEP.op_oc.operator.__init__(self,name,verb,msg)
    SEP.pj_base.par_job.__init__(self,name) 
    self.add_doc_params()


  def prep_run(self,restart=None):
    """Check to make sure parameters make sense before running"""
    self.set_operator(self.param("files")["domain"],self.param("files")["range"])
    SEP.pj_base.par_job.prep(self,restart)
                                                                                                                   
  def add_doc_params(self):
    self.run_pars=SEP.args.basic(name="runpars")
    self.run_pars.add_doc_param("adj_com","adj=y ",doc="Command line option when running adjoint",export=None)
    self.add_pars(self.run_pars)
                                                                                                                   
  def del_doc_params(self):
    """Delete the parameters associated with setting up an out of core operator"""
    for key in self.run_pars.keys(): self.del_par(key)

                                                                                                                   
  def forward_op(self,model,data,add=None,restart=None):
    """Run the forward operation
 
        model  - the model vector
        data   - the data  vector
        add    - whether or not we are adding
        restart- whether or not we are restarting

    """

    self.update_parfile_dict("domain",
      self.domain_v.clone(name=model.vec_name(),usage="INPUT",restart=restart))

    self.update_parfile_dict("range",
      self.range_v.clone(name=data.vec_name(),usage="OUTPUT",restart=restart,add=add))

    SEP.pj_base.par_job.run(self,restart)
                                                                                            
  def adjoint_op(self,model,data,add=None,restart=None):
    """Run the adjoint operation
 
        model  - the model vector
        data   - the data  vector
        add    - whether or not we are adding
        restart- whether or not we are restarting

    """
    gl=self.return_global_par_dict()
    targs=SEP.args.basic(name="adjoint")
    targs.add_pars(gl)

    targs.add_string(self.param("adj_com","adj=y"))

    self.set_global_par_dict(targs)

    self.update_parfile_dict("domain",
      self.domain_v.clone(name=model.vec_name() ,usage="OUTPUT",restart=restart,add=add))

    self.update_parfile_dict("range",
      self.range_v.clone(name=data.vec_name(),usage="INPUT",restart=restart))

    SEP.pj_base.par_job.run(self,restart)
    self.set_global_par_dict(gl)

