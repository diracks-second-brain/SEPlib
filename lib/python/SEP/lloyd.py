import SEP.sepfile
import SEP.util
import SEP.spawn
import SEP.paths
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.3.13"
                                                                                


class lloyd(SEP.opt_prog.options):
  """Class to do velocity selection  using a modified Lloyd's algorythm"""

  def __init__(self,name="Lloyd"):
    """Initialize the Lloyd velocity sellection program"""
    SEP.opt_prog.options.__init__(self,name)
    self.add_doc_params()
    self.set_program("%s/%s"%(SEP.paths.sepbindir,"Lloyd_vel"))

  def add_doc_params(self):
    """Add the parameters for the lloyd velocity selection program"""
    self.add_doc_param("verb",doc="Verbosity level",required=None)
    self.add_doc_param("nref",4,"Number of reference velocities")
    self.add_doc_param("vel_map","n","Whether or not output the selected velocity model")
    self.add_doc_param("min_region_pct",1.,"Minimum percentage of the velocity associated with given refernce" )
    self.add_doc_param("min_slow_dev",6.,"Minimum deviation percentage between reference velocityes")
    self.add_doc_param("niter_lloyd",30,"Number of non-linear iterations")
    self.add_doc_param("stdin",doc="Input sepfile")
    self.add_doc_param("stdout",doc="Output sepfile")
    
