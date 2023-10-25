import os,re,sys,string,types
import SEP.util
import SEP.spawn

__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.11.29"


class parameter:
  """Class to hold a parameter description"""
  def __init__(self,param,default=None,doc=None,required=1,value=None,export=1):
     """Initialize a parameter

        param       - the parameter descriptor
        default     - the default value for the parameter
        doc         - a description for the parameter
        required    - whether or not the parameter is required
        value       - a value for the paramter
        export      - whether or not the parameter should be exported (a command line parameter)

     """
     self.param=param
     self.default=default
     self.doc=doc
     self.required=required
     self.value=value
     self.islogic=None
     self.export=export
     self.stest=re.compile(" ")
     self.isstring=self.check_string()
     if type(self.default) is not types.NoneType: self.required=None

  def set_required(self,required):
    """Set that a parameter is required"""
    self.required=required

  def return_param(self):
    """Set the parameter name"""
    return self.param

  def set_param(self,param):
    """Set the parameter name"""
    self.param=param

  def check_string(self):
    """Check to see if a parameter is a string"""
    if type(self.value) is types.StringType:
      if self.stest.search(self.value): return 1
    return None
    

  def set_value(self,val):
    """Set the value of a parameter"""
    self.value=value
    self.check_string() 

  def required(self,p):
    """Return whether or not the parameter required"""
    if not self.param(p): 
      SEP.util.err("Can't check required status of non-defined parameter")
    return self.param[p].return_required()

     
  def clone(self,**kw):
    """Clone the parameter"""
    if kw.has_key("param"): param=kw["param"]
    else: param=self.param
    if kw.has_key("default"): default=kw["default"]
    else: default=self.default
    if kw.has_key("doc"): default=kw["doc"]
    else: doc=self.doc
    if kw.has_key("required"): doc=kw["required"]
    else: required=self.required
    if kw.has_key("value"): value=kw["value"]
    else: value=self.value
    return parameter(param,default,doc,required,value,self.export)

  def com_param(self):
    """Whether or not the parameter is a command line parameter"""
    return self.export

  def return_doc(self,prefix=""): 
     """Return a string describing the parameter"""
     if type(self.default) is not types.NoneType:
       if self.doc: 
         st="%s%s - [%s]  %s "%(prefix,self.param,
             str(self.default),self.doc)
       else:
         st="%s%s - [%s] "%(prefix,self.param,str(self.default))
     else:
       if self.doc: 
         st="%s%s - %s "%(prefix,self.param,self.doc)
       else:
         st="%s%s "%(prefix,self.param)
     if not self.required: st="%s"%st
     wds=string.split(st)
     lst=["      "]
     i=0
     for wd in wds:
       if len(lst[i]) + len(wd) >70:
         lst.append("       ")
         i=i+1
       lst[i]=lst[i]+"%s "%wd
     return lst


  def update(self,value=None,doc=None,required=None,default=None):
    """Udate the parameter structure"""
    if type(value)!=types.NoneType: self.value=value
    if doc     : self.doc     =doc
    if required: self.required=required
    if default : self.default =default


  def par(self):
    """Return the parameter value and the number of elements"""

    if type(self.value)==types.NoneType: self.value=self.default

    if type(self.value)==types.NoneType:
      return None,0
    if type(self.value)==types.ListType:
      return self.value,len(self.value) 
    return self.value,1

  def sep_string(self,quote=None):
    """Return a string sep-stylle for this parameter"""
    if quote or self.isstring:
      beg="'"
      end="'"
    else:
      beg=""
      end=""
    if type(self.value)!=types.NoneType:
      if self.param == "<" or self.param == ">": 
        return "%s%s %s%s"%(beg,self.param,self.value,end)
      else:
        if type(self.value)==types.ListType:
          v=self.value[0]
          if type(v)==types.FloatType or type(v)==types.StringType or type(v)==types.IntType:
            b=[]
            for v in self.value: b.append(str(v))
          return "%s%s=%s%s"%(beg,self.param,string.join(b,","),end)
        else: return "%s%s=%s%s"%(beg,self.param,self.value,end)

  def set_default(self,d):
    """Set the default value for this parameter"""
    self.default=d

  def return_default(self):
    """Return the default value"""
    return self.default

  def is_default(self):
    """Check to see if parameter is just set to its default"""
    if self.default == self.value: return 1
    return None

  def return_required(self):
    """Return whether the parameter is required"""
    return self.required
