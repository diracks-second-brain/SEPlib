import SEP.vec_base
import SEP.op_scale
import SEP.util
import SEP.op_base
import SEP.solv_base
import SEP.op_combo
import SEP.vec_super
import re,sys

__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.7.31"

class solver(SEP.solv_base.solver):
  def __init__(self,name):
    """Initialize a model preconditioning inversion operator
                                                                                                   
      name   - is the name associated with the solver object
    """
    SEP.solv_base.solver.__init__(self,name)


  def  prep_run(self,restart=None):
    """Build the x,rr, and total op for the inversion"""
                                                                                                   
    restart_prep,restart_iter,restart_finish=self.check_status_beg(restart)
    if restart_prep: restart=None


    d=self.param("data",error=1)
    m=self.param("model",error=1)
    prec=self.param("prec",error=1)
    op=self.param("op",error=1)
    wop=self.param("wop",None)
    eps=float(self.param("eps",error=1))
    resd=self.param("resd",None)
    resm=self.param("resm",None)
    eps=self.param("eps",error=1)


    if wop:
      rd=wop.range_v.clone_space(name=self.solve_name+"_rd")
      if restart: rd.load()
      else: 
        rd.zero()
        wop.forward(d,rd)       #rd= W d
        rd.scale(-1.)      #rd= - Wd
        if resd: rd.add(resd)
      chain_op=SEP.op_combo.chain(self.solve_name+".WLP",[prec,op,wop])
    else:
      rd=d.clone_space(name=self.solve_name+"_rd")
      if restart: rd.load()
      else: 
        rd.zero()
        rd.scale_add(-1.,d)   # rd=-d 
        if resd: rd.add(resd)
      chain_op=SEP.op_combo.chain(self.solve_name+".LP",[prec,op])

    rm=prec.domain_v.clone_space(name=self.solve_name+"_rm")
    if restart: rm.load()
    else:
      rm.zero()
      if resm: rm=rm0.add(resm)

    if not restart: m.zero()

    p=prec.domain_v.clone_space(name=self.solve_name+"_p")
    if restart: p.load()
    else:
      p.zero()
      
    self.add_param("p",p)
    scale_op=SEP.op_scale.op(self.solve_name+".eps",p,m,eps)
      

    self.vect_list=[rd,rm,p]
    rr=SEP.vec_super.vector(self.solve_name+".rr",[rd,rm])
    tot_op=SEP.op_combo.array(self.solve_name+".main",[chain_op,scale_op],1,2)
    self.add_param("solv_x",p)
    self.add_param("solv_rr",rr)
    self.add_param("solv_op",tot_op)
    SEP.solv_base.solver.prep_run(self,restart_prep,restart_iter,restart_finish)


  def solve(self):
    """Solve for a model"""
    err=self.iter()
    self.status.update_status("finish",["started"],lock=1)
#    self.prec.forward(self.param("p"),self.param("model"),self.status,-1)
    self.prec.forward(self.param("p"),self.param("model"))
    return 1
