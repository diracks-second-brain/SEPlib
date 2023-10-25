import types,string,os,re,pwd
import SEP.sepfile
import SEP.util
import SEP.datapath
import SEP.spawn
import SEP.vec_base
import SEP.log
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.3.30"

class vector(SEP.vec_base.vector):
  """ A class for out of core vectors  format"""

  def __init__(self,name):
     """Initialize a file vector

     """
     SEP.vec_base.vector.__init__(self,name)


  def file_name(self):
    """Return the file name associated with the vector"""
    return self.name
