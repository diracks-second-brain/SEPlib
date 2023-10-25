import time,os,string,types,sys
import SEP.util
import SEP.args
import SEP.vec_base
import op_oc
import SEP.pj_split
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.4.4"

class operator(op_oc.operator,SEP.pj_split.par_job):

  def __init__(self,name,verb=None,msg=None):
    """An operator class for simple coarse grain parallelization operators
                                                                                                                   
       name    - the name associated with the operator 
       verb    - whether or not to be verbose when running
       msg     - the message to print out when executing
                                                                                                                   
    """
    SEP.op_oc.operator.__init__(self,name,verb,msg)
    SEP.pj_split.par_job.__init__(self,name) 


  def prep_run(self,restart=None):
    """Check to make sure parameters make sense before running"""
    self.set_operator(self.param("files")["domain"],self.param("files")["range"])
    SEP.pj_split.par_job.prep(self,restart)
                                                                                                                   
  def add_doc_params(self):
    self.run_pars=SEP.args.basic(name="runpars")
    self.run_pars.add_doc_param("adj_com","adj=y ",doc="Command line option when running adjoint")
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
    print self.return_global_par_dict().return_set_params_string()

    self.update_parfile_dict("domain",
      self.domain_v.clone(name=model.vec_name(),usage="INPUT",restart=restart))

    if add: usage="OUTPUT"
    else: usage="INPUT"
    self.update_parfile_dict("range",
      self.range_v.clone(name=data.vec_name(),usage=usage,restart=restart,add=add))

    SEP.pj_split.par_job.run(self,restart)
    self.clean_files()
                                                                                            
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

    targs.add_string(self.param("adj_com"))

    self.set_global_par_dict(targs)

    if add: usage="OUTPUT"
    else: usage="INPUT"
    self.update_parfile_dict("domain",
      self.domain_v.clone(name=model.vec_name() ,usage=usage,restart=restart,add=add))

    self.update_parfile_dict("range",
      self.range_v.clone(name=data.vec_name(),usage="INPUT",restart=restart))

    SEP.pj_split.par_job.run(self,restart)
    self.set_global_par_dict(gl)
    self.clean_files()

