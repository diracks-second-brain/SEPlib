import types,string,os,re,pwd
import SEP.util
import SEP.datapath
import SEP.spawn
import SEP.vec_sep
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.11.30"

class vector(SEP.vec_sep.vector):
  def __init__(self,name=None,tag=None,vector=None,space_only=None):
     """Initialize a sep vector composed of complex numbers

        space_only - only initialze description don't wory about data

        Several different methods to initialize

        Initialze a vector from a file
          b=SEP.vector.scmplx(tag) 
 
        Initialze a vector from another vector
          b=SEP.vector.scmplx("aa",vector=a)
    
        Initialze an empty vector that we will define shape later
          b=SEP.vector.scmplx("aa")

     """
     SEP.vec_sep.vector.__init__(self,name,tag,vector,space_only)

  def load(self):
    """Load a sep vector file"""
    self.space_only=None
    

  def copy_data(self,vector):
     """Copy the data into this file"""
     SEP.spawn.run_wait("Cp %s %s "%(vector.vec_name(),self.vec_name()))

  def clone(self,space_only=None,name=None):
    """Clone a sep vector complex 

        space_only - only clone the space
        name       - the name for the cloned vectror

    """
    if not name: name=self.vec_name()+".clone"+str(self.clones)
    self.clones=self.clones+1
    if not name: name=self.vec_name()+".clone"+str(self.clones)
    self.clones=self.clones+1
    junk=vector(name,space_only=space_only,vector=self) 
    return junk


  def create_data(self):
     """out=0."""
     self.history.add_param("esize",8)
     self.history.add_param("in","-1")
     self.write_file()
     self.solver_func("op=zero")
     self.space_only=None

  def random(self):
     """Zero a vector"""
     if self.space_only:
       self.create_data()
     else:
       self.solver_func("op=random")

  def zero(self):
     """Zero a vector"""
     if self.space_only:
       self.create_data()
     else:
       self.solver_func("op=zero")

  def scale(self,scale):
     """out=out*scale"""
     self.solver_func("op=scale")

  def add(self,add):
     """out=out+add"""
     self.solver_func(["op=add"],add)

  def scale_addscale(self,scale1,add,scale2):
     """out=out*scale1+scale2*add"""
     if type(scale1) is types.ComplexType: 
        sc1r=scale1.real
        sc1i=scale1.imag
     else:
        sc1r=scale1
        sc1i=0.

     if type(scale2) is types.ComplexType: 
        sc2r=scale2.real
        sc2i=scale2.imag
     else:
        sc2r=scale2
        sc2i=0.
     if sc2i >= 0.: sc2i="+"+str(sc2i)
     if sc2r >= 0.: sc2r="+"+str(sc2r)
     arg=[]
     arg.extend(["op=scale_addscale", "scale1_r=%s"%str(sc1r)])
     arg.append("scale1_i=%s"%str(sc1i))
     arg.append("scale2_r=%s"%str(sc2r))
     arg.append("scale2_i=%s"%str(sc2i))
     self.solver_func(arg,add)

  def multiply(self,othervec):
     """Multiply a vector with another vector"""
     self.solver_func("op=multiply",othervec)

  def dot(self,othervec):
     """Dot product vector with  another vector """
     if self.vec_name()==othervec.vec_name(): 
       error=self.solver_func("op=dot")
     else:
       error=self.solver_func("op=dot",othervec)
     a=re.compile("\s*DOT\s+RESULT\s*\(\s*(.+)\s*,\s*(.+)\s*\)").search(error)
     if not a: SEP.util.error("Trouble with dot product")
     else: return complex(float(a.group(1)),float(a.group(2)))

  def size(self):
    """Return the size of the vector"""
    return SEP.sepfile.sep_file.size(self)
