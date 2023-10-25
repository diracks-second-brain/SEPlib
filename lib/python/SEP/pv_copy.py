import SEP.util
import SEP.vec_scmplx
import SEP.vec_sfloat
import SEP.pf_copy

__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2004.3.21"
class cmplx_vector(SEP.pf_copy.parfile,SEP.vec_scmplx.vector):
  def __init__(self,**kw):
     """Initialize a complex parallel vector


        
        Requred;
           name  - The name of the dataset 

        OPtional parameters:
          exists - Whether or not the datast exists (assumed no [None])

          vector - Another vector

          space_only - (defaults to None) means we are only describing the 
           vector space (not necessarily any data associated with it)


     """
     if not kw.has_key("name"): SEP.util.err("Must provide the name of the dataset")
     name=kw["name"]
     exists=None
     if kw.has_key("exists"): exists=kw["exists"]
     vector=None 
     if kw.has_key("vector"): vector=kw["vector"]
     space_only=None
     if kw.has_key("space_only"): space_only=kw["space_only"]

     if exists:  
       tag=name
       name=None
     else: tag=None

     SEP.vec_scmplx.vector.__init__(self,name,tag,vector,space_only)

                                                                                    
     if vector:
       cc=SEP.pv_func.clean_vec_keywords(vector.kw,["space_only","load"])
       cc["name"]=name
       for d,v in kw.items(): cc[d]=v
       SEP.pf_copy.parfile.__init__(self,space_only=space_only,**cc)
     else:
       SEP.pf_copy.parfile.__init__(self,**kw)
       if kw.has_key("load") and kw["load"]: self.restart(1)

     if self.kw.has_key("exists"): del self.kw["exists"]
     if exists:
       a=SEP.sepfile.sep_file(file=tag)
#       a=self.sepfile_from_sects()
       for iax in range(a.ndims()):
         n,o,d,label,unit=a.axis(iax+1)
         self.set_axis(iax+1,n,o,d,label,unit)



  def axis(self,iax):
    return SEP.sepfile.sep_file.axis(self,iax)

  def clone(self,**kw):
    """Clone a parallel file object (copy)"""
    kout={}
    for key,val in self.kw.items(): kout[key]=val
    for key,val in kw.items(): kout[key]=val
    if not kw.has_key("space_only") and kout.has_key("space_only"):  
      del kout["space_only"]
    if not kout.has_key("name") or not kout["name"]: kout["name"]="%s.copy"%self.vec_name()
    a= cmplx_vector(**kout)
    for iax in range(self.ndims()):
      n,o,d,label,unit=self.axis(iax+1)
      a.set_axis(iax+1,n,o,d,label,unit)
    a.history.add_param("esize",8)
#    if not kout.has_key("space_only"):
#      a.write_file()
#      SEP.vec_scmplx.vector.copy_data(a,self)
    return a


class float_vector(SEP.pf_copy.parfile,SEP.vec_sfloat.vector):
  def __init__(self,**kw):
     """Initialize a complex parallel vector


        
        Requred;
           name  - The name of the dataset 

        OPtional parameters:
          exists - Whether or not the datast exists (assumed no [None])

          vector - Another vector

          space_only - (defaults to None) means we are only describing the 
           vector space (not necessarily any data associated with it)


     """
     if not kw.has_key("name"): SEP.util.err("Must provide the name of the dataset")
     name=kw["name"]
     exists=None
     if kw.has_key("exists"): exists=kw["exists"]
     vector=None 
     if kw.has_key("vector"): vector=kw["vector"]
     space_only=None
     if kw.has_key("space_only"): space_only=kw["space_only"]

     if exists:  
       tag=name
       name=None
     else: tag=None

        
     SEP.vec_sfloat.vector.__init__(self,name,tag,vector,space_only)

     if vector:
       cc=SEP.pv_func.clean_vec_keywords(vector.kw,["space_only","load"])
       cc["name"]=name
       for d,v in kw.items(): cc[d]=v
       SEP.pf_copy.parfile.__init__(self,space_only=space_only,**cc)
     else:
       SEP.pf_copy.parfile.__init__(self,**kw)
       if kw.has_key("load") and kw["load"]: self.restart(1)

     if self.kw.has_key("exists"): del self.kw["exists"]
     if exists:
       a=SEP.sepfile.sep_file(file=tag)
#       a=self.sepfile_from_sects()
       for iax in range(a.ndims()):
         n,o,d,label,unit=a.axis(iax+1)
         self.set_axis(iax+1,n,o,d,label,unit)

  def axis(self,iax):
    return SEP.sepfile.sep_file.axis(self,iax)

  def clone(self,**kw):
    """Clone a parallel file object (copy)"""
    kout={}
    for key,val in self.kw.items(): kout[key]=val
    for key,val in kw.items(): kout[key]=val
    if not kw.has_key("space_only") and kout.has_key("space_only"):  
      del kout["space_only"]
    if not kout.has_key("name") or not kout["name"]: kout["name"]="%s.copy"%self.vec_name()
    a= float_vector(**kout)
    for iax in range(self.ndims()):
      n,o,d,label,unit=self.axis(iax+1)
      a.set_axis(iax+1,n,o,d,label,unit)
#    if not kout.has_key("space_only"):
#      a.write_file()
#      SEP.vec_scmplx.vector.copy_data(a,self)
    return a

