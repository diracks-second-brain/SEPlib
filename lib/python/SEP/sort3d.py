import SEP.sepfile
import SEP.util
import SEP.spawn
import types
import string
import SEP.opt_prog
import SEP.paths
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.04.12"


class  sort3d(SEP.opt_prog.options):
  def __init__(self,name="Sort3d"):
    """Initialize the Sort3d  program"""
    SEP.opt_prog.options.__init__(self,name)
    self.add_doc_params()
    self.set_program("%s/%s"%(SEP.paths.sepbindir,"Sort3d"))
 
  def add_doc_params(self):
    """Parameters for Sort3d"""
    
    self.add_doc_param("nkeys",doc="Number of keys")
    self.add_doc_param("plane",doc="plane to transpose (eg) '34'",required=None)
    self.add_doc_param("verb",0,doc="Whether or not to be verbose")
    self.add_doc_param("synch",0,doc=" [0] whether [1] or not [0] to synch traces")
    self.add_doc_param("compress_tr",0,doc=" [1] whether (1) or not(0) to compress the trace in bin axis if it equal 1")
    self.add_doc_param("preserve_nd",0,doc="[0]  wheter (1) or not(0) to preserver the input n2,n3,n4,n... values")
    self.add_doc_param("max_size",200.,doc="Maximum memory to use")
    self.add_doc_param("stdin",doc="Input file")
    self.add_doc_param("stdout",doc="Output file")
    self.add_doc_param("key1",doc="First key to sort on",required=None)
    self.add_doc_param("key2",doc="Key 2 to sort on (required if nkeys>1",required=None)
    self.add_doc_param("key3",doc="Key 3 to sort on (required if nkeys>2",required=None)
    self.add_doc_param("key4",doc="Key 4 to sort on (required if nkeys>3",required=None)
    self.add_doc_param("key5",doc="Key 5 to sort on (required if nkeys>4",required=None)
    self.add_doc_param("key6",doc="Key 6 to sort on (required if nkeys>5",required=None)
    self.add_doc_param("ng",doc="Number of grid cells for axes related to keys")
    self.add_doc_param("og",doc="First grid cell location for axes related to keys")
    self.add_doc_param("dg",doc="Sampling of grid cells for axes related to keys")

  def prep_run(self,restart=None):  
    """Check to make sure the parameters make sense"""

    ins=SEP.sepfile.sep_file(name=self.param("stdin"))

    plane=self.param("plane",error=None)
      
    if plane: self.build_transp_params(ins,plane)
    else: self.build_sort_params(ins)
    SEP.opt_prog.options.prep_run(self,restart)


  def build_sort_params(self,ins):
    """Build parameters to do a Sort"""
   
    nkeys=int(self.param("nkeys"))

    ng=self.param("ng")
    og=self.param("og")
    dg=self.param("dg")
    if nkeys != len(ng):
      print "nkeys=%d  ng="%nkeys,ng
      SEP.util.err("Expecting ng to be length of nkeys")
  
    if nkeys != len(og):
      print "nkeys=%d  ng="%nkeys,ng
      SEP.util.err("Expecting og to be length of nkeys")
  
    if nkeys != len(dg):
      print "nkeys=%d  ng="%nkeys,ng
      SEP.util.err("Expecting dg to be length of nkeys")
  
    for i in range(nkeys):
      ikey=i+1
      self.param("key%d"%ikey)
      self.add_param("ng%d"%ikey,ng[i])
      self.add_param("og%d"%ikey,og[i])
      self.add_param("dg%d"%ikey,dg[i])
    self.del_par("ng")
    self.del_par("og")
    self.del_par("dg")

  def build_transp_params(self,ins,plane):
    """Build parameters to do a transpose"""

    if not ins.grid:
      SEP.util.err("Attempted transpose with non-gridded data")

    if len(plane)!=2: 
      SEP.util.err("Plane should be in form plane=23")

    axis1=int(plane[0]) 
    axis2=int(plane[1]) 
  
    ndims=int(ins.ndims())
    label=self.axis(2)[3]

    if label == "trace_in_bin": imin=3
    else: imin=2

    if axis1 < imin or axis1 > ndims:
      SEP.util.err("Transp axis 1 out of range %d <= %d < %d"%(imin,axis1,
        ndims))
        
    if axis2 < imin or axis2 > ndims:
      SEP.util.err("Transp axis 2 out of range %d <= %d < %d"%(imin,axis2,
        ndims))

