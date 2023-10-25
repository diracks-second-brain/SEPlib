import SEP.sepfile
import SEP.util
import SEP.spawn
import SEP.paths
import SEP.opt_prog

__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.3.12"
                                                                                


class transp(SEP.opt_prog):
  """Class to do tranpose"""

  def __init__(self):
    """Initialize the Transpose"""
    SEP.opt_prog.options.__init__(self,"Transp")
    self.add_doc_params()
    self.set_program("%s/%s"%(SEP.paths.sepbindir,"Transp"))

  def add_doc_params(self):
    """Add the parameters for the lloyd velocity selection program"""
    self.add_doc_param("plane","12","Plane to transpose")
    self.add_doc_param("maxsize",100,"Approximate memory to use")
    self.add_doc_param("verb",doc="Whether to be verbose (0 or 1)",required=None)
    self.add_doc_param("reshape",doc="Treat the cube dimensions differently")
    self.add_doc_param("stdin",doc="Input sepfile")
    self.add_doc_param("stdout",doc="Output sepfile")
