import os
import sys
import string
import SEP.util
import SEP.spawn
import SEP.args
import types

__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.4.12"
                                                                                
class options(SEP.args.basic):
  """A group of parameters """

  def __init__(self,name):
     """Initialize a class for parameters """
     SEP.args.basic.__init__(self,name=name)
     self.my_name=name
     self.prog_opt={}
     self.local_dict={}
     self.add_doc_params()
     self.prefix=""

  def set_prefix(self,prefix):
    self.prefix=prefix


  def add_doc_params(self):
    """Add parameters for this module"""
                                                                                
  def del_doc_param(self,par):
     """Delete a parameter from the self-doc"""
     self.del_par(par)
                                                                                
  def add_doc_param(self,par,default=None,doc=None,required=1):
    """Add a parameter to the self doc
                                                                                
        par         - the parameter descriptor
        default     - the default value for the parameter
        doc         - a description for the parameter
                                                                                
    """
    self.add_param(par,None,doc=doc,default=default,required=required)


  def clean_files(self):
    """Clean files"""

  def read_params(self,args,delete=None):
      """Read the parameters for the par object

         args -  the program arguments to check against (SEP.args.basic)
         delete- whether or not to delete the parameters from args that we need

         return err,args

           err        - whether (1) or not (None) all arguments were set
           args       - the command line args, possibly without arguments from this group
      
      """
      error=None
      
      main_pars=args.extract_pars(self.prefix)
      for p,arg in self.params.items():
        val,err=main_pars.par(p,error=None)
        if err!=0: 
          self.add_param(p,val=val)
          args.del_par("%s%s"%(self.prefix,p))
        else:
          val,err=self.par(p,error=None)
          if err==0 and self.return_required(p):
            error=1
            SEP.util.msg( "ERROR(%s): %s \n"%(self.my_name,p))
            lines=self.return_doc()
            for line in lines: SEP.util.msg(line)
      return error,args

  def build_check_params(self):
    """Build program parameters based on other program parameters"""

  def prep_run(self,restart=None):
    """Check to make sure everything is setup before running"""
    SEP.util.err("Must override prep_run")

  def set_default(self,par,default):
     """Set the default for a parameter in this parameter group"""
     if not self.par_list.has_key(par):
        SEP.util.err("Attempt to set default %s which doesn't exist "%par)
     self.par_list[par].set_default(default)

  def run_in_flow(self):
    """Return whether a option is runable"""
    return 1

  def return_doc(self):
     """Return the documentation for this section"""
     require=[]
     defaulted=[]
     option=[]
                                                                                
     for par,struct in self.params.items():
        if  struct.com_param():
          if type(struct.default) is not types.NoneType : defaulted.append(par)
          elif struct.required: require.append(par)
          else:  option.append(par)
     doc=[]
     if require: doc.extend(["     Required"])
     for par in require: doc.extend(self.params[par].return_doc(self.prefix))
     if option: doc.extend(["     Optional"])
     for par in option : doc.extend(self.params[par].return_doc(self.prefix))
     if defaulted: doc.extend(["     Defaulted"])
     for par in defaulted : doc.extend(self.params[par].return_doc(self.prefix))
     return doc
