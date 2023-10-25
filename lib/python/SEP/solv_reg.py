import SEP.vec_base
import SEP.util
import SEP.op_scale
import SEP.op_combo
import re,sys
import SEP.vec_super
import solv_base

__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.7.29"
                                                                                
class solver(solv_base.solver):
  def __init__(self,name):
    """Initialize a simple inversion operator
     
      name   - is the name associated with the solver object

    """
    SEP.solv_base.solver.__init__(self,name)


  def  prep_run(self,restart=None):
    """Build the x,rr, and total op for the inversion"""

    restart_prep,restart_iter,restart_finish=self.check_status_beg(restart)
    if restart_prep: restart=None
    m=self.param("model",error=1)
    d=self.param("data",error=1)
    eps=real(self.param("eps",error=1))
    resd=self.param("resd",None)
    resm=self.param("resm",None)
    m0=self.param("m0",None)
    op=self.param("op",error=1)
    wop=self.param("wop",None)
    reg=self.param("reg",error=1)
                                                                                                   

    if wop:
      rd=wop.range_v.clone_space(name=self.solve_name+"_rd")
      if restart: rd.load()
      else: 
        rd.zero()
        wop.forward(d,rd)       #rd= W d
        rd.scale(-1.)      #rd= - Wd
        if resd: rd.add(resd)
      chain_op=SEP.op_combo.chain(self.solve_name+".WO",[op,wop])
    else:
      rd=d.clone_space(name=self.solve_name+"_rd") # rd=d
      if restart: rd.load()
      else: 
        rd.zero()  
        rd.scale_add(-1.,d) #rd=-d
        if resd: rd.add(resd)
      chain_op=op

    rm=reg.range_v.clone_space(name=self.solve_name+"_rm")
    if  restart: rm.load()
    else:
      rm.zero()
      if resm: rm.add(resm)
    self.vect_list=[rm,rd]

    scale_op=SEP.op_scale.op("eps_reg",m,m,eps)
    reg_chain=SEP.op_combo.chain("eA",[reg,scale_op])

    if not restart: m.zero()
    if m0 and not restart:
      m.add(m0)    # m=m0
      chain_op(m,rd,add=1)
      reg_chain(m,rm,add=1)
      
    rr=SEP.vec_super.vector(self.solve_name+".rr",[rd,rm])
    tot_opt=SEP.op_combo.array(self.solve_name+".main",[chain_op,reg_chain],1,2)
    self.add_param("solv_x",m)
    self.add_param("solv_rr",rr)
    self.add_param("solv_op",tot_op)
    SEP.solv_base.solver.prep_run(self,restart_prep,restart_iter,restart_finish)
     
  def solve(self):
    """Iterate the problem"""
    if not self.iter(): return None
    return 1
