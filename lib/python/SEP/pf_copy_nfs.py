import SEP.sep_socket
import SEP.sepfile
import os,sys,types,commands,string,re
import SEP.util
import SEP.datapath
import SEP.paths
import SEP.pf_copy
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2004.3.18"


class parfile(SEP.pf_copy.parfile):

  def __init__(self,**kw):
    """Initialze a copy paralel file (same space on all nodes) that
        is stored on a nfs disk
    

       Optional parameters:
       share_datapath -  the datapath that the image file are store on (datapath)
       nmax_procs     -  the maximum number of processes that share a file
       extra_args     -  additional arguments that need to be added with this dataset

       SEE parent class for more parameters

    """
    SEP.pf_copy.parfile.__init__(self,**kw)
    self.dpath=self._option("share_datapath",SEP.datapath.datapath())
    self.datafile=self.dpath+os.path.basename(self.file_name)
    self.nmax=int(self._option("nmax_procs",8)  )
    self.big_args=self._option("extra_args","")
    self.nshare=[]
    for i in range(10000/self.nmax): self.nshare.append(0)

  def clone(self,**kw):
    """Clone a parallel file object of copy share"""
    kout=self.kw
    for key,val in kw.items(): kout[key]=val
    if not kw.has_key("space_only") and kout.has_key("space_only"):  
      del kout["space_only"]
    a=parfile(**kout)
    return a

  def _datapath(self,mach):
    """Reeturn the datapath for the  output file"""
    return self.dpath

  def _section_name(self,key,mlabel):
    """Get the file name if we have distributed a file

        key   - the section name
        mlabel- the machine label

        returns section_name,section_name if we have a new section
        returns section_name,None if the section alread exists


    """
    for i in range(10000/self.nmax): 
      file="%s_%d"%(self.datafile,i)
      if self.nshare[i] <self.nmax:
         self.nshare[i]=self.nshare[i]+1
         if self.get_elem_val(2,file):
           return file,None
         if self.nshare[i] ==1: return file,file
         return file,None

  def done(self,key):
    """Mark that a key is finished"""
    parfile.done(self,key)
    sect=self.get_key_status_elem(key,2)
    ab=re.compile("%s_(.+)"%self.datafile).search(sect)
    if not ab:
      SEP.util.error("Trouble finding %s in %s "%(self.datafile,sect))
    self.nshare[int(ab.group(1))]=self.nshare[int(ab.group(1))]-1

  def collect(self,key_list,extra_args=[" "]):
     """Collect a dataset

          key_list is the list of keys to collect

     """
     if self.usage != "OUTPUT": return
     pars,machs,nbytes=self._collect_pars(key_list)
     if self.verb >1: SEP.util.msg("Collecting  %s from %s"%(self.file_name,
        string.join(machs)))
     sects=[]
     for key in key_list:
       sect=self.get_key_status_elem(key,2)
       if sects.count(sect)==0: sects.append(sect)
     t=SEP.util.temp_file()
     SEP.spawn.run_wait("Add %s >%s %s"%(string.join(sects),self.file_name,
       self.big_args),logfile=t) 
     SEP.spawn.run_wait("rm -f  %s %s_* &"%(t,self.datafile),60)

  def tags(self,key_label):
    """Get local section names

        key_label is a dictionry linking key names and machine labels
        Return is a dicitonary linking key names and tags

        If the file is  "INPUT" the data will be distributed to the nodes if needed
        If the file is being reused or restarted appropriate parameters will be added

    """
    err,tags=parfile.tags(self,key_label) 
    tout={}
    if not err and tags:
      for key,tag in tags.items(): tout[key]="%s %s"%(tag,self.big_args)
    return err,tout

