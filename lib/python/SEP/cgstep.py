import SEP.vec_base
import sys
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2004.8.1"
                                                                                


def dot_problem(dot1,dot2):
    """Give an error when doing dot product"""
    SEP.util.msg("Zero dot product result when dotting %s and %s"%(dot1,dot2))
    return 1

class cgstep:
  """Conjugate-gradient step """

  def __init__(self,name):
    """Initialize a cgstep object

       name- ascii string to identify cgstep

    """
    self.name=name
    self.eps=.000001

  def alloc(self,mod,dat,restart):
    """allocate space (or load) previous step 

       mod     - a vector of the shape of the model
       dat     - a vector of the shape of the residual
       restart - whether or not we are restarting the inversion

       return None if successful, 1 if error 

    """
    self.s=mod.clone_space(name=self.name+"_s")
    self.ss=dat.clone_space(name=self.name+"_ss")
    self.tempm=mod.clone_space(name=self.name+"_tempm")
    self.tempd=dat.clone_space(name=self.name+"_tempd")
    if restart:
      self.s.load()
      self.ss.load()
      self.tempm.load()
      self.tempd.load()
    else:
      self.s.zero()
      self.ss.zero()
      self.tempm.zero()
      self.tempd.zero()
    self.forget="yes"

  def job_desc(self,iter):
    """Return the job desription info for a given step"""
    return(["%d.calc_len.step"%iter,"%d.add_s.step"%iter,"%d.add_x.step"%iter,
      "%d.add_ss.step"%iter,"%d.add_rr.step"%iter])
    

  def set_forget(self,a):
    """set the status of the forget variable 

        if a is true we do steepest descent
        if a is None do conjugate gradient step

    """ 
    self.forget=a

  def clean(self):
    """clean the vectors neeeded for the conjugate gradient """
    self.ss.clean()
    self.s.clean()
    self.tempm.clean()
    self.tempd.clean()

  def calc_what_to_do(self,status,iter,restart):
    names=self.job_desc(iter)
    to_do=[]
    to_record=[]
    copy_from=[]
    copy_to=[]
    to_do=[1,1,1,1,1]
    copy_from=[0,0,0,0]
    if status: 
       to_record=[1,1,1,1,1]
       copy_to=[1,1,1,1] 
    else:
        to_record=[0,0,0,0,0]
        copy_to=[0,0,0,0] 
    if restart:
      if restart==names[1]: #if restarting add_s
        copy_from[0]=1 
      if restart==names[2]: #if restarting add_x
        copy_from[1]=1 
        to_do[1]=0
      if restart==names[3]: #if restarting add_s
        copy_from[2]=1 
        to_do[0]=0;to_do[1]=0;to_do[2]=0
      if restart==names[4]: #if restarting add_s
        copy_from[3]=1 
        to_do[0]=0;to_do[1]=0;to_do[2]=0;to_do[3]=0
    return  names,to_do,to_record,copy_from,copy_to
   

  def step(self,x,g,rr,gg,iter=0,status=None,restart=None):
    if restart and name!=restart: return restart

    """Perform a conjugate gradient step


        x  - model vector
        g  - gradient vector
        gg - L g vector
        rr - residual vector

    """



    names,to_do,to_record,copy_from,copy_to=self.calc_what_to_do(status,
       iter,restart)
    if to_do[0]:
      if to_record[0]: status.update_status(names[0],["started"],lock=1)
      if self.forget:
        if to_do.count(0)==0:
          self.s.zero()
          self.ss.zero()
        beta=0.
        gdot=gg.dot(gg)
        if abs(gdot)< self.eps : return dot_problem("gg","gg")
        gr=gg.dot(rr)
        alfa=-gr/gdot
        sys.stdout.flush()
        self.forget=None
      else:
        gdg=gg.dot(gg); #search plane by solving 2-by-2
        sds=self.ss.dot(self.ss);  # G . (R - G*alfa - S*beta) = 0
        gds=gg.dot(self.ss);   #S . (R - G*alfa - S*beta) = 0
        sdg=self.ss.dot(gg);   #S . (R - G*alfa - S*beta) = 0
        if not gdg: return dot_problem("gg","gg")
        if not sds: return dot_problem("ss","gg")
        determ=abs(gdg*sds - gds*sdg)
        gdr=gg.dot(rr)
        sdr=self.ss.dot(rr)
        alfa= -( sds * gdr - gds * sdr ) / determ;
        beta =- (-gds * gdr + gdg * sdr ) / determ;
      if to_record[0]: status.update_status(names[0],["finished"],lock=1)
    

    print "ALPHA BETA",alfa,beta
    sys.stdout.flush()

    if to_do[1]:
      if to_record[1]: status.update_status(names[1],["started"],lock=1)
      if copy_from[0]: self.s.scale_addscale(0.,self.tempm,1.)
      self.s.scale_addscale(beta,g,alfa)       #update model step
      if copy_to[0]: self.tempm.scale_addscale(0.,self.s,1.)
      if to_record[2]: status.update_status(names[1],["finished"],lock=1)
      

    if to_do[2]:
      if to_record[2]: status.update_status(names[2],["started"],lock=1)
      if copy_from[1]: x.scale_addscale(0.,self.tempm,1.)
      x.add(self.s)    #update solution
      if copy_to[1]: self.tempm.scale_addscale(0.,x,1.)
      if to_record[2]: status.update_status(names[2],["finished"],lock=1)

    if to_do[3]:
      if to_record[3]: status.update_status(names[3],["started"],lock=1)
      if copy_from[2]: self.ss.scale_addscale(0.,self.tempd,1.)
      self.ss.scale_addscale(beta,gg,alfa)     #update residual step
      if copy_to[2]: self.tempd.scale_addscale(0.,self.ss,1.)
      if to_record[3]: status.update_status(names[3],["finished"],lock=1)

    if to_do[4]:
      if to_record[4]: status.update_status(names[4],["started"],lock=1)
      if copy_from[3]: rr.scale_addscale(0.,self.tempd,1.)
      rr.add(self.ss)  #update residual
      if copy_to[3]: self.tempd.scale_addscale(0.,rr,1.)
      if to_record[4]: status.update_status(names[4],["finished"],lock=1)

    return None

