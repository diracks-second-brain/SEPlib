import SEP.vec_base
import SEP.op_combo
import SEP.util
import SEP.op_scale
import re,sys
import SEP.opt_base
import SEP.solv_base

__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.8.23"
                                                                                
class solver(SEP.solv_base.solve_core):
  def __init__(self,name):
    """Initialize a simple inversion operator
     
      name   - is the name associated with the solver object

    """
    SEP.solv_base.solve_core.__init__(self,name)

  def clean_files(self):
    """Clean the solver """
    for vec in self.vect_list: vec.clean()
    self.solv_op.clean()

  def read_solve_params(self):
    """Read in parameters"""
    self.solv_x=self.m
    self.solv_rr=self.rr
    self.solv_op=self.tot_op
    self.solv_niter=int(self.param("niter",error=1))
    self.solv_verb=self.param("verb",None)
    self.solv_iter0=0

  def construct_vectors(self,restart_iter):
    self.solv_g=self.solv_x.clone_space(name=self.solve_name+"_g")
    self.solv_p=self.solv_x.clone_space(name=self.solve_name+"_p")
    self.solv_gp=self.solv_x.clone_space(name=self.solve_name+"_gp")
    self.solv_gx=self.solv_x.clone_space(name=self.solve_name+"_gx")
    self.solv_sp=self.solv_x.clone_space(name=self.solve_name+"_sp")
    self.solv_sx=self.solv_x.clone_space(name=self.solve_name+"_sx")

    self.solv_gr=self.solv_rr.clone_space(name=self.solve_name+"_gr")
    self.solv_sr=self.solv_rr.clone_space(name=self.solve_name+"_sr")

    if not restart_iter:
      self.solv_g.zero()
      self.solv_p.zero()
      self.solv_gp.zero()
      self.solv_gx.zero()
      self.solv_sp.zero()
      self.solv_sx.zero()
      self.solv_gr.zero()
      self.solv_sr.zero()
    else:
      self.solv_g.load()
      self.solv_p.load()
      self.solv_gp.load()
      self.solv_gx.load()
      self.solv_sp.load()
      self.solv_sx.load()
      self.solv_gr.load()
      self.solv_sr.load()

    self.vect_list.extend([self.solv_g,self.solv_p,self.solv_gp,self.solv_gx])
    self.vect_list.extend([self.solv_sp,self.solv_sx,self.solv_gr,self.solv_sr])
     



  def  prep_run(self,restart=None):
    """Build the x,rr, and total op for the inversion"""

    restart_prep,restart_iter,restart_finish=self.check_status_beg(restart)
    if restart_prep: restart=None
    self.m=self.param("model",error=1)
    d=self.param("data",error=1)
    resd=self.param("resd",None)
    resm=self.param("resm",None)
    m0=self.param("m0",None)
    op=self.param("op",error=1)
    wop=self.param("wop",None)
    self.shape=self.param("shape",error=1)
                                                                                                   

    if wop:
      rr=wop.range_v.clone_space(name=self.solve_name+"_rr")
      if restart: rr.load()
      else: 
        rr.zero()
        wop.forward(d,rr)       #rd= W d
        rr.scale(-1.)      #rd= - Wd
        if resd: rd.add(resd)
      self.tot_op=SEP.op_combo.chain(self.solve_name+".WO",[op,wop])
    else:
      self.rr=d.clone_space(name=self.solve_name+"_rr") # rd=d
      if restart: self.rr.load()
      else: 
        self.rr.zero()  
        self.rr.scale_add(-1.,d) #rd=-d
        if resd: self.rr.add(resd)
      self.tot_op=op

    self.vect_list=[self.rr]


    if not restart: self.m.zero()
    if m0 and not restart:
      self.m.add(m0)    # m=m0
      self.tot_op(self.m,rd,add=1)
      
    if restart_prep or restart_iter or restart_finish: restart=1
    else: restart=None

    SEP.solv_base.solve_core.prep_run(self,restart_prep,restart_iter,restart_finish)

  def do_fshape(self,base,iter,model,data,restart=None,add=None):
    SEP.log.logging.set_prefix("%s%s.%d.fshape."%(base,self.solve_name,iter))
    self.shape.forward(model,data,self.status,iter,restart=restart,add=add)
    SEP.log.logging.set_prefix(base)

  def do_ashape(self,base,iter,model,data,restart=None,add=None):
    SEP.log.logging.set_prefix("%s%s.%d.ashape."%(base,self.solve_name,iter))
    self.shape.adjoint(model,data,self.status,iter,restart=restart,add=add)
    SEP.log.logging.set_prefix(base)


  def restart_it(self,restart_list):
    """Restart the solver"""
    #update status
    if restart_list:
      iterS=re.compile("^(\d+)\.").search(restart_list[0])
      if  not iterS: SEP.util.err("Unknown status parameter %s"%restart_list[0])
      iter=int(iterS.group(1))
      self.solv_iter0=iter+1
      base=self.get_base()
      if   re.compile("_start$").search(restart_list[0]): step=0
      elif re.compile("_a$").search(restart_list[0]): step=1
      elif re.compile("_f$").search(restart_list[0]): step=2
      elif re.compile("st$").search(restart_list[0]): step=3
      elif re.compile("extra$").search(restart_list[0]): step=4
      else: SEP.util.err("unknown parameter %s"%restart_list[0]);
      if step==0: self.iter_beg(base,iter,restart_list[0])
      if step<=1: 
         self.do_adjoint(base,iter,self.solv_gx,self.solv_rr,restart=restart_list[0],add=1)
         self.do_ashape(base,iter,self.solv_gp,self.solv_gx,restart=restart_list[0],add=1)
      if step<=2:  
         self.do_fshape(base,iter,self.solv_gp,self.solv_gx,restart=restart_list[0])
         self.do_forward(base,iter,self.solv_gx,self.solv_gr,restart=restart_list[0])
      if step<=3:
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
      job_list.append(str(iter)+".start")
      f,a=self.solv_op.job_desc(iter)
      job_list.extend(a)
      fs,as=self.shape.job_desc(iter)
      job_list.extend(as)
      job_list.extend(fs)
      job_list.extend(f)
      job_list.append(str(iter)+".step")
      job_list.append(str(iter)+".extra")
    job_list.append("finish")
    self.status=SEP.stat_sep.status(self.solve_name+".stat",job_list)
    self.status._unlock_it()
                                                                                     
                                                                                     
  def do_step(self,base,iter,restart=None):
    """Do a step"""
    SEP.log.logging.set_prefix("%s%s.%d.step."%(base,self.solve_name,iter))
    self.status.update_status("%s.step"%str(iter),["started"],lock=1)
    gn=self.solv_gp.dot(self.solv_gp)
    e=float(self.param("eps") )
    if int(iter) ==0:
      g0=gn
      b0=abs(gn+e*e*abs(self.solv_gr.dot(self.solv_gr) -self.solv_gx.dot(self.solv_gx)))
      self.solv_sp.scale_addscale(0.,self.solv_gp,1.)
      self.solv_sx.scale_addscale(0.,self.solv_gx,1.)
      self.solv_sr.scale_addscale(0.,self.solv_gr,1.)
    else:
      alpha=gn/self.gnp
      #dg=gn/g0
      self.solv_sp.scale_addscale(alpha,self.solv_gp,1.)
      self.solv_sx.scale_addscale(alpha,self.solv_gx,1.)
      self.solv_sr.scale_addscale(alpha,self.solv_gr,1.)
    inner=self.solv_sp.dot(self.solv_sp)-self.solv_sx.dot(self.solv_sx)
    beta=self.solv_sr.dot(self.solv_sr)+e*e*inner
    alpha=-gn/beta
    self.solv_p.scale_addscale(1.,self.solv_sp,alpha)
    self.solv_x.scale_addscale(1.,self.solv_sx,alpha)
    self.solv_rr.scale_addscale(1.,self.solv_sr,alpha)
    self.gnp=gn
    self.status.update_status("%s.step"%str(iter),["finished"],lock=1)

  def do_begin(self,base,iter,restart=None):
    """Init vectors for this iteration"""
    e=float(self.param("eps") )
    self.solv_gp.scale_addscale(0.,self.solv_p,e*e)
    self.solv_gx.scale_addscale(0.,self.solv_x,-e*e)

  def iter(self):
    """Perform one iteration of the iterative solver"""
    base=self.get_base()
    for iter in range(self.solv_iter0,self.solv_niter):
      self.do_begin(base,iter)
      self.do_adjoint(base,iter,self.solv_gx,self.solv_rr,add=1)
      self.do_ashape(base,iter,self.solv_gp,self.solv_gx,add=1)
      self.do_fshape(base,iter,self.solv_gp,self.solv_gx)
      self.do_forward(base,iter,self.solv_gx,self.solv_gr)
      self.do_step(base,iter)
      self.do_extra(base,iter)
      if self.solv_verb>0: self.report(iter)
    SEP.log.logging.set_prefix(base)
    return None
                                                                                     

