import SEP.vec_base
import SEP.util
import SEP.op_scale
import SEP.op_combo
import re,sys
import SEP.solv_base

__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.7.31"

class solver(SEP.solv_base.solver):
  """A  class for simple inversions d= Lm"""
  def __init__(self,name):
    """Initialize a simple inversion operator
     
      name   - is the name associated with the solver object
    """
    SEP.solv_base.solver.__init__(self,name)


  def  prep_run(self,restart=None):
    """Build the x and rr for the inversion"""

    restart_prep,restart_iter,restart_finish=self.check_status_beg(restart)
    if restart_prep: restart=None
    m=self.param("model",error=1)
    d=self.param("data",error=1)
    op=self.param("op",error=1)
    wop=self.param("wop",None)
    resd=self.param("resd",None)
    m0=self.param("m0",None)
    

    if wop:
      rr=wop.range_v.clone_space(name=self.solve_name+".smp_rr")
      if restart: rr.load()
      else:
        rr.zero()
        wop.forward(d,rr)  #rr= W d
        rr.scale(-1.)      #rr= - Wd
        if resd: rr.add(resd)
      chain_op=SEP.op_combo.chain(self.solve_name+".WL",[op,wop],restart,verb=self.param("verb"))
    else:
      rr=d.clone_space(name=self.solve_name+".smp_rr") # rr=d
      if  restart: rr.load()
      else:
        rr.zero()
        rr.scale_add(-1.,d)   # rr=-d 
        if resd: rr.add(resd)
      chain_op=op
    self.vect_list=[rr]

    if m0 and not restart:
      m.zero()
      m.add(m0)
      chain_op(m,rr,add="yes")


    self.add_param("solv_x",m)
    self.add_param("solv_rr",rr)
    self.add_param("solv_op",chain_op)
    SEP.solv_base.solver.prep_run(self,restart_prep,restart_iter,restart_finish)

  def solve(self):
    """Solve for the model"""
    if not self.iter(): return None
    return 1

