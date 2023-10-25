import sys
import SEP.util
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.3.13"
                                                                                


class vector:
  """A generic vector class"""

  def __init__(self,name):
     """Initialze a vector (must be overridden)"""
     self._vec_name=name
     

  def vec_name(self):
    """Return the vector name"""
    return self._vec_name


  def clone(self,space_only=None):
    """Clone a vector (must be overridden)"""
    SEP.util.err( "Calling unimplemtned method   ")

  def clone_space(self):
    """Clone the space of a vector """
    return self.clone(space_only=1)

  def zero(self):
     """Zero a vector (must override)"""
     SEP.util.err( "Calling umimplented method")

  def random(self):
     """Put random  numbers into vector(must override)"""
     SEP.util.err( "Calling umimplented method")

  def scale(self,scale):
     """out=out*scale (must override)"""
     SEP.util.err ( "Calling umimplented method")

  def add(self,add):
     """out=out+add (must override)"""
     SEP.util.err ( "Calling umimplented method")

  def scale_add(self,scale,add):
     """out=out+scale*add (must override)"""
     self.scale_addscale(0.,add,scale)

  def scale_addscale(self,scale1,add,scale2):
     """out=out*scale1+scale2*add (must override)"""
     SEP.util.err ( "Calling umimplented method")

  def multiply(self,othervec):
     """Multiply a vector with another vector (must override)"""
     SEP.util.err("Calling umimplented method")

  def dot(self,othervec):
     """Dot product vector with another vector (must override) """
     SEP.util.err( "Calling umimplented method")

  def load(self,name):
    """Load a vector [for restarting] (must override)"""
    SEP.util.err ("Load vector not defined")

  def size(self):
     """Return the size for simple logic checking (must override)"""
     SEP.util.err( "Calling umimplented method")

  def clean(self):
     """Clean a vector (must override)"""
     SEP.util.err( "Calling umimplented method")
