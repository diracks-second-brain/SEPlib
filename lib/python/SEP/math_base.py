import SEP.sepfile
import SEP.util
import SEP.spawn
import types
import string
import SEP.paths
import SEP.parameter
import SEP.opt_prog
import SEP.pj_base
import SEP.paths
import SEP.pf_split
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.04.10"
def math_docs():
  par_list=SEP.args.basic(name="AAA")
  par_list.add_doc_param("exp",doc="Expression for real output",required=None)
  par_list.add_doc_param("exp_real",doc="Expression for real output",required=None)
  par_list.add_doc_param("exp_image",doc="Expression for real output",required=None)
  par_list.add_doc_param("file1",doc="File 1",required=None)
  par_list.add_doc_param("file2",doc="File 2",required=None)
  par_list.add_doc_param("file3",doc="File 3",required=None)
  par_list.add_doc_param("file4",doc="File 4",required=None)
  par_list.add_doc_param("file5",doc="File 5",required=None)
  par_list.add_doc_param("file6",doc="File 6",required=None)
  par_list.add_doc_param("file7",doc="File 7",required=None)
  par_list.add_doc_param("file8",doc="File 8",required=None)
  par_list.add_doc_param("file9",doc="File 9",required=None)
  par_list.add_doc_param("verb","n",doc="Whether or not to be verbose")
  par_list.add_doc_param("maxsize",200,doc="Maximum memory to use in megabytes")
  return par_list

class math(SEP.opt_prog.options):
  """Class to do math"""

  def __init__(self,name="math"):
    """Initialize the Math program"""
    SEP.opt_prog.options.__init__(self,name)
    self.add_doc_params()
    self.set_program("%s/%s"%(SEP.paths.sepbindir,"Math_base"))

  def add_doc_params(self):
    """Parameters for Math"""
    self.add_pars(math_docs())
    self.add_doc_param("stdout",doc="Output SEP dataset")

class math_clean(math):
  """Math without the docs"""
  def __init__(self,name="math"):
    """Initialize the Math program"""
    math.__init__(self,name)
    self.add_doc_params()

  def add_doc_params(self):
    """Parameters for Math"""
    math.add_doc_params(self)
    self.del_doc_param("file1")
    self.del_doc_param("file2")
    self.del_doc_param("file3")
    self.del_doc_param("file4")
    self.del_doc_param("file5")
    self.del_doc_param("file6")
    self.del_doc_param("file7")
    self.del_doc_param("file8")
    self.del_doc_param("file9")
    self.del_doc_param("exp")
    self.del_doc_param("exp_real")
    self.del_doc_param("exp_image")

