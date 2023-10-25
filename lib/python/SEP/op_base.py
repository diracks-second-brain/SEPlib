import SEP.util
import SEP.prog
import time,os,string,types,sys
import op_base
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.4.4"
                                                                                

def check_space(vec1,vec2):
    """Check to see if two vectors are of the same type and size """
    if type(vec1)!=type(vec2):
      SEP.util.msg("Vectors not of the same type")
      return 1
    if vec1.size()!=vec2.size():
      SEP.util.msg("Vectors not the same size %s %s "%(vec1.size(),vec2.size()))
      return 1
    return None


class operator:
  """A generic operator class"""

  def __init__(self,name,domain,range,**kw):
    """Initialize a generic operator
    
       name    - ASCII string describing operator (required)
       domain  - The domain vector for the operator (required)
       range   - The range vector for the operator  (required)
       verb    - Whether or not to be verbose when running operator (0)
       msg     - The message to print when running (defaults to name)

    """
    self.domain_v=domain.clone_space("%s.domain"%name)
    self.range_v=range.clone_space("%s.range"%name)
    self.kw=kw
    self.name_op=name
    self.verb=self._option("verb",0)
    self.nsteps=1
    self.msg=self._option("msg",name)

  def op_name(self):
    """Return the operator name """
    return self.name_op

  def _option(self,par,default=None):
    """Return an optional paramter if it exists, else None"""
    if not self.kw.has_key(par):  return default
    return self.kw[par]

  def range_vec(self):
    """Return the range vector"""
    return self.range_v

  def domain_vec(self):
    """Return the domain vector"""
    return self.domain_v

  def set_verb(self,verb):
    self.verb=verb

  def adjoint(self,d,r,status=None,iter=-1,add=None,restart=None):
    """Run adjoint operation

       d       -   domain vector
       r       -   range vector
       status  -   status object to update when running (optional)
       iter    -   Iteration we are on (optional=0)
       add     -   Whether or not we are adding
       restart -   Whether or not we are restarting

    """

    name="%d.%s_a"%(iter,self.op_name())
    if restart and name!=restart: return restart
    self.check_operator_logic(d,r)
    if status: status.update_status(name,["started"],lock=1)
    if self.verb: SEP.util.msg( "%d iter adjoint operation %s"%(iter,self.msg))
    self.adjoint_op(d,r,add,restart)
    if status: status.update_status(name,["finished"],lock=1)
    if  restart: return None
    return restart

  def forward(self,d,r,status=None,iter=-1,add=None,restart=None):
    """Run forward operation

       d       -   domain vector
       r       -   range vector
       status  -   status object to update when running (optional)
       iter    -   Iteration we are on (optional=0)
       add     -   Whether or not we are adding
       restart -   Whether or not we are restarting

    """

    self.check_operator_logic(d,r)
    name="%d.%s_f"%(iter,self.op_name())
    if restart and name!=restart: return restart
    if self.verb: SEP.util.msg( "%d iter forward operation %s"%(iter,self.msg))
    if status: status.update_status(name,["started"],lock=1)
    self.forward_op(d,r,add,restart)
    if status: status.update_status(name,["finished"],lock=1)
    if  restart: return None
    return restart

  def init_op(self,restart):
    """Initialize the operator"""

  def adjoint_op(self,model,data,add,restart):
    """Function to run the adjoint operation"""
    SEP.util.err("Must define the adjoint operation")

  def forward_op(self,model,data,add,restart):
    """Function to run the forward operation """
    SEP.util.err("Must define the forward operation")
    
  def job_desc(self,iter):
    """Return job description for an operator"""
    return ["%d.%s_f"%(iter,self.op_name())], ["%d.%s_a"%(iter,self.op_name())]
    
  def check_operator_logic(self,d,r):
    """Check to make sure the doman and range vector are correct for the operator"""
    if op_base.check_space(self.domain_v,d):
       SEP.util.err("Domain vector %s not the correct space for operator %s"%(d.vec_name(),self.op_name()))
    if op_base.check_space(self.range_v,r):
       SEP.util.err("Range vector %s not the correct space for operator %s"%(d.vec_name(),self.op_name()))


  def clean(self):
    """Perform any cleaning needed for this operation"""

  def dot_test(self):
     """Check to see if the operator passes the dot product test"""
     mod1=self.domain_v.clone_space(name="dot1_mod")
     mod2=self.domain_v.clone_space(name="dot2_mod")
     dat1=self.range_v.clone_space(name="dot1_dat")
     dat2=self.range_v.clone_space(name="dot2_dat")
     dat1.zero();dat2.zero();dat2.random()
     mod1.zero();mod2.zero();mod1.random()
     self.init_op(None)
     dot1=[];dot2=[]
     self.forward(mod1,dat1);dot1.append(dat1.dot(dat2))
     self.adjoint(mod2,dat2);dot1.append(mod1.dot(mod2))
     err=0
     if abs(dot1[0])==0.: 
       SEP.util.msg("Dot product is 0")
       err=1 
     if(abs((dot1[0]-dot1[1])/dot1[0]) >.001):
       SEP.util.msg("Failed dot product %s %s "%(dot1[0],dot1[1]));
       err=1 
     self.forward(mod1,dat1,add=1); dot2.append(dat1.dot(dat2))
     self.adjoint(mod2,dat2,add=1); dot2.append(mod1.dot(mod2))
     if abs(dot2[0])==0:
       SEP.util.msg("Dot product is 0")
       err=1 
     if(abs((dot2[0]-dot2[1])/dot2[0]) >.001):
       SEP.util.msg("Failed add dot product %s %s "%(dot2[0],dot2[1]));
       err=1 
     if not err:
       SEP.util.msg("Passed dot product!")
       return None
     else: return 1


  def nops(self):
    """Return the number of operators this operator is composed of"""
    return self.nsteps 
