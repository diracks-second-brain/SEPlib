import SEP.util
import SEP.vec_base
import SEP.prog
import time,os,string,types,sys
import op_base
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.4.4"


class op(op_base.operator):
  """A simple operator that does scaling by a scalar """
  def __init__(self,name,domain,range,scale,restart=None,verb=0):
     """Initialize a scale operator

       name     - the name for the operator
       domain   - the domain vector
       range    - the range vector
       scale    - the scale
       restart  - whether we are restarting
       verb     - whether or not to be verbose
       msg      - the message to print when running 

     """
     if domain.size() != range.size():
       SEP.util.err("domain and range must be the same size for scale_op")
     self.scale=scale
     op_base.operator.__init__(self,name,domain,range,verb=verb)
     
  def adjoint_op(self,d,r,add,restart=None):
    """Run the adjoint scaling operation
 
        model  - the model vector
        data   - the data  vector
        add    - whether or not we are adding
        restart- whether or not we are restarting

    """
    if add: sc=1.
    else: sc=0.
    d.scale_addscale(sc,r,self.scale)

  def forward_op(self,d,r,add,restart=None):
    """Run the scaling forward operation
 
        model  - the model vector
        data   - the data  vector
        add    - whether or not we are adding
        restart- whether or not we are restarting

    """
    if add: sc=1.
    else: sc=0.
    r.scale_addscale(sc,d,self.scale)

