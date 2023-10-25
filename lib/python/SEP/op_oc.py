import SEP.util
import SEP.vec_base
import SEP.prog
import time,os,string,types,sys
import SEP.op_base
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.4.4"
                                                                                
class operator(SEP.op_base.operator):
  """An operator class for out of core operations"""

  def __init__(self,name,verb=None,msg=None):
    """Initialize a simple out of core operator 
 
       name    - the name associated with the operator 
       verb    - verbosity level
       msg     - message to print when executing

    """
    kw={}
    self.domain_v=None
    self.range_v=None
    self.myname=name
    self.verb=verb
    self.msg=msg
    
  

  def range_vec(self):
    """Return the range vector"""
    if not self.range_v: SEP.util.err("Range vector has not been set")
    return self.range_v

  def domain_vec(self):
    """Return the domain vector"""
    if not self.domain_v: SEP.util.err("Domain vector has not been set")
    return self.domain_v


  def run_in_flow(self):
    return None
  def set_operator(self,domain,range):
    """Setup operator"""
    SEP.op_base.operator.__init__(self,self.myname,domain,range,verb=self.verb,msg=self.msg)

