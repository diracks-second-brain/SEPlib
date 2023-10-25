import sys
import vec_base
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.3.21"
                                                                                
class vector(vec_base.vector):
  """A vector of vectors"""

  def __init__(self,name,vec_list):
     """Initialize a super vector from a list of vectors"""
     self.name=name
     self.vecs=vec_list

  def load(self):
     """ Load a s super vector"""
     for vec in self.vecs: vec.load()

  def clone(self,name=None,space_only=None):
     """ Clone a super vector
      
         name       - the name for the super vector
         space_only - only clone the space of the super vector

     """
     vec_list=[]
     i=0
     if not name: name=self.name+"_clone"
     for vec in self.vecs:
       vec_list.append(vec.clone(name=name+str(i),space_only=space_only))
       i+=1
     junk=vector(name,vec_list)
     return junk

  def clone_space(self,name):
    """ Clone just the space of the vector """
    return self.clone(name,space_only=1)

  def zero(self):
     """Zero a vector"""
     for vect in self.vecs: vect.zero()

  def random(self):
     """Put random  numbers into vector"""
     for vect in self.vecs: vect.random()

  def scale(self,scale):
     """out=out*scale"""
     for vect in self.vecs: vect.scale(scale)

  def add(self,add):
     """out=out+add"""
     if len(self.vecs) != len(add.vecs):
       SEP.util.err("incompatible vectors to scale_addscale")
     for i in range(len(self.vecs)):
       self.vecs[i].add(add.vecs[i])


  def scale_addscale(self,scale1,add,scale2):
     """out=out*scale1+scale2*add"""
     if len(self.vecs) != len(add.vecs):
       SEP.util.err("incompatible vectors to scale_addscale")
     for i in range(len(self.vecs)):
       self.vecs[i].scale_addscale(scale1,add.vecs[i],scale2)

  def multiply(self,othervec):
     """Multiply a vector with another vector"""
     if len(self.vecs) != len(othervec.vecs):
       SEP.util.err("incompatible vectors to multiply")
     for i in range(len(self.vecs)):
       self.vecs[i].multuply(othervec.vecs[i])

  def dot(self,othervec):
     """Dot product vector with  another vectory """
     d_list=[]
     if len(othervec.vecs) != len(self.vecs):
       SEP.util.err("Attempt to dot incompatible vectors %s and %s"%(self.name,othervec.name))
     for i in range(len(self.vecs)):
       d_list.append(self.vecs[i].dot(othervec.vecs[i]))
     dtot=0
     for d in d_list: dtot+=d
     return dtot

  def size(self):
    """Return the size of the super vector"""
    n=0
    for vect in self.vecs: 
      n=int(n)+int(vect.size())
    return n

  def clean(self):
    """Clean a super vector"""
    for vect in self.vecs:vect.clean()
