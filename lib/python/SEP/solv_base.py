import SEP.vec_base
import SEP.util
import SEP.opt_base
import SEP.op_combo
import SEP.stat_sep
import re,sys,os

__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.7.31"
                                                                                


class solve_core(SEP.opt_base.options):
  def __init__(self,name):
    """A generic solver object. 

       All other linear solvers are derived from this class

    """
    SEP.opt_base.options.__init__(self,name)
    self.solve_name=name

  def clean_files(self):
    """Clean the solver """
#    for vec in self.vect_list: vec.clean()
#    self.solv_op.clean()

  def prep_run(self,restart_prep,restart_iter,restart_finish):
    """Prepare and run inverseion"""
    if restart_prep or restart_iter or restart_finish: restart=1
    else: restart=None

    self.solv_stepper=None

    self.read_solve_params()
    if not restart or restart_prep: self.calc_work()
    self.construct_vectors(restart_iter)
    if self.solv_stepper: 
      r=None
      if restart_finish or restart_iter: r=1
      self.solv_stepper.alloc(self.solv_x,self.solv_rr,r)

    self.status.update_status("prep",["finished"],lock=1)
    self.solv_op.init_op(restart)
    self.restart_it(restart_iter)
    self.solve()

  def solve(self):
    """Solve problem"""
    err=self.iter()
    if err: SEP.util.err("Trouble iterating")


  def do_forward(self,base,iter,model,data,restart=None,add=None):
    SEP.log.logging.set_prefix("%s%s.%d.forward."%(base,self.solve_name,iter))
    self.solv_op.forward(model,data,self.status,iter,restart=restart,add=add)
    SEP.log.logging.set_prefix(base)

  def do_adjoint(self,base,iter,model,data,restart=None,add=None):
    SEP.log.logging.set_prefix("%s%s.%d.adjoint."%(base,self.solve_name,iter))
    self.solv_op.adjoint(model,data,self.status,iter,restart=restart,add=add)
    SEP.log.logging.set_prefix(base)

  def check_status_beg(self,restart):
    restart_prep=None
    restart_finish=None
    restart_iter=None
    restart_list=[]
    if restart:
      if os.path.isfile(SEP.log.logging.add_dir(self.solve_name+".stat")):
        self.status=SEP.stat_sep.status(self.solve_name+".stat",load=1)
        restart_list=self.status.restart(restart,["finished"],["started","restart"])
        self.status._unlock_it()
        if restart_list[0]=="prep": restart_prep=1
        elif restart_list[0]=="finish": restart_finish=1
        else: restart_iter=restart_list 
      else: restart_prep=1
    
    return restart_prep,restart_list,restart_finish


  def do_extra(self,base,iter):
    """Allow additional calculations at the end of an iteration"""
    SEP.log.logging.set_prefix("%s%s.%d.extra."%(base,self.solve_name,iter))
    self.status.update_status(str(iter)+".extra",["started"],lock=1)
    self.end_iter(iter)
    self.status.update_status(str(iter)+".extra",["finished"],lock=1)
    return 1

  def end_iter(self,iter):
    """Do any additional work at the end of an iteration"""

  def report(self,iter):
    """Report the energy in varios vectors"""
    dd=self.solv_rr.dot(self.solv_rr)
    dx=self.solv_x.dot(self.solv_x)
    dg=self.solv_g.dot(self.solv_g)
    if iter==0: 
      self.dx=dx.real/1000. 
      self.dg=dg.real/1000. 
    SEP.util.msg( "iter=%d r(dat)=%f r(x)=%f r(grad)=%f"%(iter,dd.real/self.dd,
      dx.real/self.dx,dg.real/self.dg))


  def get_base(self):
    base2=SEP.log.logging.return_prefix()
    if base2: base=base2+"."
    else: base=""
    return base
    


class solver(solve_core):
  """A generic solver object for standard step methods


   """
  def __init__(self,name):
    solve_core.__init__(self,name)
    self.solve_name=name

  def clean_files(self):
    """Clean the solver """
#    solve_core.clean_files(self)
#    self.solv_stepper.clean()
  

  def construct_vectors(self,restart_iter):
    self.solv_g=self.solv_x.clone_space(name=self.solve_name+"_g")
    self.solv_gg=self.solv_rr.clone_space(name=self.solve_name+"_gg")
    if not restart_iter:
      self.solv_g.zero()
      self.solv_gg.zero()
    else:
      self.solv_g.load()
      self.solv_gg.load()
    self.vect_list.extend([self.solv_g,self.solv_gg])

  def read_solve_params(self):
    """Read  in parameters"""
    self.solv_x=self.param("solv_x",error=1)
    self.solv_rr=self.param("solv_rr",error=1)
    self.solv_op=self.param("solv_op",error=1)
    self.solv_niter=int(self.param("niter",error=1))
    self.solv_verb=self.param("verb",None)
    self.solv_stepper=self.param("stepper",error=1)
    self.solv_iter0=0

  def prep_run(self,restart_prep,restart_iter,restart_finish):
    """Prepare and run inverseion"""
    solve_core.prep_run(self,restart_prep,restart_iter,restart_finish)



  def restart_it(self,restart_list):
    """Restart the solver"""
    #update status
    if restart_list:
      iterS=re.compile("^(\d+)\.").search(restart_list[0])
      if  not iterS: SEP.util.err("Unknown status parameter %s"%restart_list[0])
      iter=int(iterS.group(1))
      self.solv_iter0=iter+1
      base=self.get_base()
      if re.compile("_a$").search(restart_list[0]): step=0
      elif re.compile("_f$").search(restart_list[0]): step=1
      elif re.compile("step$").search(restart_list[0]): step=2
      elif re.compile("extra$").search(restart_list[0]): step=3
      else: SEP.util.err("unknown parameter %s"%restart_list[0]);
      if iter >0: self.solv_stepper.set_forget(None)
      if step==0: self.do_adjoint(base,iter,self.solv_g,self.solv_rr,restart_list[0])
      if step<=1: self.do_forward(base,iter,self.solv_g,self.solv_gg,restart_list[0])
      if step<=2: 
        if not self.do_step(base,iter,restart_list[0]): return None
      if not self.do_extra(base,iter): return None


  def calc_work(self):
    """Calculate what has to be done for the inversion 

       This information is used for restarting

        basically makes a list of the form

        op.0.adj
        op.0.forward
        op.0.cgstep
        ....

    """

    job_list=["prep"]
    for iter in range(self.solv_niter):
      f,a=self.solv_op.job_desc(iter)
      job_list.extend(a)
      job_list.extend(f)
      job_list.extend(self.solv_stepper.job_desc(iter))
      job_list.append(str(iter)+".extra")
    job_list.append("finish")
    self.status=SEP.stat_sep.status(self.solve_name+".stat",job_list)
    self.status._unlock_it()
        

  def do_step(self,base,iter,restart=None):
    """Do a step"""
    SEP.log.logging.set_prefix("%s%s.%d.step."%(base,self.solve_name,iter))
    if not self.solv_stepper.step(self.solv_x,self.solv_g,self.solv_rr,
       self.solv_gg,iter,self.status,restart): return None
    return 1



  def iter(self):
    """Perform one iteration of the iterative solver"""
    base=self.get_base()
    for iter in range(self.solv_iter0,self.solv_niter):
      self.do_adjoint(base,iter,self.solv_g,self.solv_rr)
      self.do_forward(base,iter,self.solv_g,self.solv_gg)
      if self.do_step(base,iter):
        SEP.util.err("Steop problem")
      self.do_extra(base,iter)
      if self.solv_verb>0: self.report(iter) 
    SEP.log.logging.set_prefix(base)
    return None


