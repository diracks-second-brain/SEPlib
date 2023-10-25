import SEP.sep_socket
import SEP.sepfile
import os,sys,types,commands,string,re
import SEP.util
import SEP.datapath
import SEP.paths
import SEP.pf_split
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.3.12"
class parfile(SEP.pf_split.parfile):
   """A class where the data is already on disk, but split along another axis/axes


      Required:
        old_file  -  The status file containing how the data is spread accross the nodes
      Optional:
        remove_old -  Whether or not to clean the old file when job completes


      See parent classes for more options

   """
   def __init__(self,**kw):

     self.kw=kw
     name=self._option("name",err=1)
     if self._option("usage",err=1) != "INPUT":
       SEP.util.err("A predistributed file can only be input")
     kw_send={}
     for  k,v in kw.items(): kw_send[k]=v
     kw_send["restart"]=1
     kw_send["load"]=1
     kw_send["name"]=self._option("old_file",err=1)
     kw_send["remove"]=self._option("remove_old",1)
     self.par_file=SEP.pf_split.parfile(**kw_send)
     self.par_file.restart(1)
     SEP.pf_split.parfile.__init__(self,**kw)

                                                                                
 

   def clean_parts(self):
     """Clean up the file parts"""
     SEP.pf_split.parfile.clean_parts(self)
     self.par_file.clean_parts(self)

   def _distributed_pars(self,pars,mach_ithread,machs):
     """Add parameters if the file is distributed"""
     pars,machs,mach_ithread=self.par_file.add_pars_all_sections(self.par_file.return_keys(),pars,"in_",machs,mach_ithread)
     pars.append("input_begin=%d"%(len(machs)+1))
     return pars,mach_ithread,machs

   def input_size(self):
     """Return the size of the input data"""
     key=self.par_file.return_keys()[0]
     return self.par_file.sect_sepfile(key).size()*8
