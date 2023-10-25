import SEP.uitl
import SEP.vec_scmplx
import SEP.vec_sfloat
import SEP.pf_copy_nfs

__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2004.3.21"
class cmplx_vector(SEP.pf_copy_nfs.parfile,SEP.vec_scmplx.vector):
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
       SEP.pf_copy_nfs.parfile.__init__(self,space_only=space_only,**vector.kw)
     else:
       SEP.pf_copy_nfs.parfile.__init__(self,**kw)
  def clone(self,**kw):
    """Clone a parallel file object (copy)"""
    kout={}
    for key,val in self.kw.items(): kout[key]=val
    for key,val in kw.items(): kout[key]=val
    if not kw.has_key("space_only") and kout.has_key("space_only"):  
      del kout["space_only"]
    if not kout.has_key("name") or not kout["name"]: kout["name"]="%s.copy"%self.vec_name()
    return cmplx_vector(**kout)

class float_vector(SEP.pf_copy_nfs.parfile,SEP.vec_sfloat.vector):
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
       SEP.pf_copy_nfs.parfile.__init__(self,space_only=space_only,**vector.kw)
     else:
       SEP.pf_copy_nfs.parfile.__init__(self,**kw)

  def clone(self,**kw):
    """Clone a parallel file object (copy)"""
    kout={}
    for key,val in self.kw.items(): kout[key]=val
    for key,val in kw.items(): kout[key]=val
    if not kw.has_key("space_only") and kout.has_key("space_only"):  
      del kout["space_only"]
    if not kout.has_key("name") or not kout["name"]: kout["name"]="%s.copy"%self.file_name()
    return float_vector(**kout)

