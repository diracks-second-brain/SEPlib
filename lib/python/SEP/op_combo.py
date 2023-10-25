import SEP.util
import SEP.vec_base
import SEP.prog
import time,os,string,types,sys
import op_base
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.12.1"

class array(op_base.operator):
  """Complex array operator

       With this class you can build operator arrays 

        d   L 
          =    m
        0   A

  """
  def __init__(self,name,array,nrow,ncol,verb=0,msg=None):
    """Initialize an array operator 

        name   - the name associated with the operator
        array  - a vector of operators
        ncol   - number of collumns in operator
        verb   - whether or not to be verbose
        msg    - Message (defaulting to name) to print when running operator

        Example:

         d   L        op=SEP.op_combo.array("reg",[L,A]a,1,2)
           =    m
         0   A


    """ 


    if len(array) != nrow*ncol: 
      SEP.util.err("Error: nrow=%d ncol=%d len(array)"%(nrow,ncol,len(array)))

    self.op_list=array
    self.nrow=nrow
    self.ncol=ncol
    self.nop=len(array)

    self.domain_list=[]
    for irow in range(self.nrow):  self.domain_list.append(None)

    self.range_list=[]
    for icol in range(self.ncol):  self.range_list.append(None)

    i=0
    for icol in range(ncol):
      for irow in range(nrow):
        if self.op_list[i]:
          if self.range_list[icol]: 
            if op_base.check_space(self.range_list[icol],self.op_list[i].range_vec()):
              SEP.util.err("range vectors not equiv")
          else: self.range_list[icol]=self.op_list[i].range_vec().clone_space()
          if self.domain_list[irow]: 
            if op_base.check_space(self.domain_list[irow],self.op_list[i].domain_vec()):
              SEP.util.err("domain vectors not equiv")
          else: self.domain_list[irow]=self.op_list[i].domain_v.clone_space()
        i=i+1

    for irow in range(nrow): 
      if not self.domain_list[irow]: SEP.util.err("Undefined domain vector %d"%irow)

    for icol in range(ncol): 
      if not self.range_list[icol]: SEP.util.err("Undefined range vector %d"%icol)
    
    if ncol >1: range_v=SEP.vec_super.vector(name+".range",self.range_list)
    else: range_v=self.range_list[0].clone_space()

    if nrow >1: domain_v=SEP.vec_super.vector(name+".domain",self.domain_list)
    else: domain_v=self.domain_list[0].clone_space()
    kw={}
    kw["msg"]=msg
    kw["verb"]=verb
    SEP.op_base.operator.__init__(self,name,domain_v,range_v,**kw)


  def job_desc(self,iter):
    """Return job description for a matrix operator"""
    list_f=[]
    list_a=[]
    for i in range(0,self.nop,1):
      a,b=self.op_list[i].job_desc(iter)
      list_f.extend(a)
    for i in range(self.nop-1,-1,-1):
      a,b=self.op_list[i].job_desc(iter)
      list_a.extend(b)
    return list_f,list_a
    

  def forward(self,d,r,status=None,iter=0,add=None,restart=None):
    """Run forward operation for an array operator

       d       -   domain vector
       r       -   range vector
       status  -   status object to update when running (optional)
       iter    -   Iteration we are on (optional=0)
       add     -   Whether or not we are adding
       restart -   Whether or not we are restarting

    """
    if self.verb: SEP.util.msg( "Forward array operation %s"%self.msg)

    for icol in range(self.ncol):
      addit=add
      for irow in range(self.nrow):
        if self.ncol >1:
          if self.nrow >1:
            restart=self.op_list[self.nrow*icol+irow].forward(d.vecs[irow],r.vecs[icol],
             status, iter,add=addit,restart=restart)
          else:
            restart=self.op_list[self.nrow*icol+irow].forward(d,r.vecs[icol],
             status, iter,add=addit,restart=restart)
        else:
          if self.nrow >1:
            restart=self.op_list[self.nrow*icol+irow].forward(d.vecs[irow],r,
             status, iter,add=addit,restart=restart)
          else:
            restart=self.op_list[self.nrow*icol+irow].forward(d,r,
             status, iter,add=addit,restart=restart)
        addit=1
    return restart
    
  def adjoint(self,d,r,status=None,iter=0,add=None,restart=None,stat=1):
    """Run adjoint operation for an array operator

       d       -   domain vector
       r       -   range vector
       status  -   status object to update when running (optional)
       iter    -   Iteration we are on (optional=0)
       add     -   Whether or not we are adding
       restart -   Whether or not we are restarting

    """
    if self.verb: SEP.util.msg( "Adjoint array operation %s"%self.msg)
    for irow in range(self.nrow-1,-1,-1):
      addit=add
      for icol in range(self.ncol-1,-1,-1):
        if self.ncol >1:
          if self.nrow >1:
            restart=self.op_list[self.nrow*icol+irow].adjoint(d.vecs[irow],r.vecs[icol],
              status, iter,add=addit,restart=restart)
          else:
            restart=self.op_list[self.nrow*icol+irow].adjoint(d,r.vecs[icol],
              status, iter,add=addit,restart=restart)
        else:
          if self.nrow >1:
            restart=self.op_list[self.nrow*icol+irow].adjoint(d.vecs[irow],r,
              status, iter,add=addit,restart=restart)
          else:
            restart=self.op_list[self.nrow*icol+irow].adjoint(d,r,
              status, iter,add=addit,restart=restart)
        addit=1
    return restart
    
 
  def init_op(self,restart):
    """Initialize an array operator"""
    for op in self.op_list: op.init_op(restart)

  def clean(self):
    """Clean an array opeator"""
    if self.verb: SEP.util.msg("Clean matrix operation %s"%self.msg)
    for op in  self.op_list: op.clean()


class chain(SEP.op_base.operator):
  """A class for chaining together operators"""

  def __init__(self,name,op_list,verb=0, msg=None):
    """Initialize a chain operator

       name     - the name for the operator
       op_list  - the list of operators to chain together
       verb     - whether or not to be verbose
       msg      - the message to print when running 

       Example:

          d=LBp
         op=SEP.op_combo.chain("prec",[L,B])

    """

    self.op_list=op_list
    self.inter_vec=[]
    d=self.op_list[0].domain_v.clone_space()
    r=self.op_list[len(self.op_list)-1].range_v.clone_space()
    self.nsteps=0
    self.nop=len(self.op_list)
    for op in op_list: self.nsteps+=op.nops()

    for i in range(1,len(self.op_list)):
      rlen=self.op_list[i-1].range_v.size()
      dlen=self.op_list[i].domain_v.size()
      if rlen!=dlen:
        SEP.util.err("domain and range vector mismatch for chain_op")
      self.inter_vec.append(self.op_list[i].domain_v.clone_space(name=name+str(i)))
    SEP.op_base.operator.__init__(self,name,d,r,msg=msg,verb=verb)
      

  def job_desc(self,iter):
    """Return job description for a chain operator"""
    list_f=[]
    list_a=[]
    for i in range(0,self.nop,1):
      a,b=self.op_list[i].job_desc(iter)
      list_f.extend(a)
    for i in range(self.nop-1,-1,-1):
      a,b=self.op_list[i].job_desc(iter)
      list_a.extend(b)
    return list_f,list_a
    

  def forward(self,d,r,status=None,iter=-1,add=None,restart=None):
    """Run adjoint operation for an chain operator

       d       -   domain vector
       r       -   range vector
       status  -   status object to update when running (optional)
       iter    -   Iteration we are on (optional=0)
       add     -   Whether or not we are adding
       restart -   Whether or not we are restarting

    """
    if self.verb: SEP.util.msg("%s iter forward row operation %s"%(str(iter),self.msg))
    restart=self.op_list[0].forward(d,self.inter_vec[0],status,iter,restart=restart)
    for i in range(0,self.nop-2):
      restart=self.op_list[i+1].forward(self.inter_vec[i],self.inter_vec[i+1],status,iter,restart=restart)
    restart=self.op_list[self.nop-1].forward(self.inter_vec[self.nop-2],r,status,iter,add=add,restart=restart)
    
  def adjoint(self,d,r,status=None,iter=-1,add=None,restart=None):
    """Run adjoint operation for an chain operator

       d       -   domain vector
       r       -   range vector
       status  -   status object to update when running (optional)
       iter    -   Iteration we are on (optional=0)
       add     -   Whether or not we are adding
       restart -   Whether or not we are restarting

    """
    if self.verb: SEP.util.msg( "%s iter adjoint row operation %s"%(str(iter),self.msg))
    restart=self.op_list[self.nop-1].adjoint(self.inter_vec[len(self.inter_vec)-1],r,status,iter,restart=restart)
    for i in range(self.nop-3,-1,-1):
      restart=self.op_list[i+1].adjoint(self.inter_vec[i],self.inter_vec[i+1],status,iter,restart=restart)
    restart=self.op_list[0].adjoint(d,self.inter_vec[0],status,iter,add,restart=restart)

  def clean(self):
    """Clean a chain operator (delete intermediate space)"""
    if self.verb: SEP.util.msg( "Clean row operation %s"%self.msg)
    for op in  self.op_list: op.clean()
    for vec in self.inter_vec: 
      vec.clean()

  def init_op(self,restart):
    """Initialize a chain operator"""
    for op in self.op_list: op.init_op(restart)
    for vec in self.inter_vec: 
      if not restart: vec.zero()
      else: vec.load()
