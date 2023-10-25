import os
import sys
import string
import SEP.util
import SEP.spawn
import SEP.args
import types
import SEP.opt_base

__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.4.12"
                                                                                
class options(SEP.opt_base.options):
  """A group of parameters """

  def __init__(self,name):
     """Initialize a class for parameters """
     SEP.opt_base.options.__init__(self,name=name)
                                                                                
  def prep_run(self,restart=None):
    """No need to prep_run when options not assoicated with program or parallel jobs"""

  def run_in_flow(self):
     return None

