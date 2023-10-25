import os
import sys
import string
import SEP.args
import SEP.util
import SEP.spawn
import types
import flow

__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.3.21"
                                                                                
class prog(SEP.flow.flow):
  """A class for documenting the options in a program"""


  def __init__(self,summary,usage,par_blocks=[],description=[],comments=[],examples=[],
     args=None,name=None,flows=[],prefixes={},restartable=None,order=[]):
    """Initialize a program object

       summary    - A one line summary for the program
       usage      - Example usage
       par_blocks - A list of par_groups that make up the arguments
       description- A list describing the program
       comments   - A list describing comments for the program
       examples   - A list with examples for the
       args       - The arguments for the program  (defaults to sys.argv)
       name       - The name of the program   (defaults to sys.argv[0])
       prefixes   - A dictionary with  par_block.name:prefix defaults to "" for all
       restartable- Whether or not to make the program restartable
       order      - The order to run things. Defaults to flows then par_blocks

    """
       
    self.summary   =summary
    self.usage     = usage
    self.description=description
    self.comments   =comments
    self.examples   =examples
    self.par_dict={}
    if args: 
      self.args=args
    else:
      self.args=SEP.args.sep_args()
    self.orig_args=SEP.args.basic(name="orig")
    self.orig_args.add_pars(self.args)

    if self.args.param("print_all",delete=1,error=None): SEP.spawn.print_all=1
    if self.args.param("print_flow",delete=1,error=None): SEP.flow.print_flow=1
    if self.args.param("debug_py",delete=1,error=None): SEP.util.debug=1
    self.restart=self.args.param("restart",error=None)

    if not name: name=os.path.basename(sys.argv[0])
    flow.flow.__init__(self,name=name,par_blocks=par_blocks,prefixes=prefixes,flows=flows,restartable=restartable,order=order)
    self.prefixes=prefixes

    if len(sys.argv) ==1:
      d=self.doc()
      for line in d: print "%s "%line
      sys.exit(-1)


  def return_orig_args(self):
    """Return the original argument list"""
    return self.orig_args

  def add_description(self,des):
    """Add a line to to the description for the program"""
    self.description.append(des)
  def add_comment(self,com):
    """Add a comment line to the program"""
    self.comments.append(com)
  def add_example(self,examp):
    """Add a line to the examples for the program"""
    self.examples.append(examp)

  def get_options(self,delete=None):
     """Read/build the parameters for the program"""
     self.par_dict={}
     self.par_dict,self.args=SEP.flow.flow.get_options(self,self.par_dict,self.args,delete)

  def return_prog_args(self):
    """Return the arguments for the all par groups"""
    total_args=SEP.args.basic(name=self.my_name)
    for block in self.par_blocks:
       total_args.add_pars(block.return_basic_params())
    return total_args

  def doc(self):
    """Return the documentation for the program"""
    lines=["%s -  %s"%(self.my_name,self.summary),""]
    lines.extend(["USAGE","  %s"%self.usage])
    lines.extend(["PARAMETERS"])

    lines.extend(self.doc_pars())
    if self.description:
      lines.extend(["DESCRIPTION",""])
      for line in self.description: lines.append(line)
    
    if self.examples:
      lines.extend(["EXAMPLES",""])
      for line in self.examples: lines.append(line)
    
    if self.comments:
      lines.extend(["COMMENTS",""])
      for line in self.comments: lines.append(line)
    return lines

