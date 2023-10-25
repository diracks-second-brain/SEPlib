import SEP.sep_socket
import SEP.sepfile
import os,sys,types,commands,string,re
import SEP.util
import SEP.datapath
import SEP.paths
import SEP.pf_base
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2004.4.6"


class parfile(SEP.pf_base.parfile):
  def __init__(self,**kw):
    """Initialze a copy paralel file (same space on all nodes)
    

       Optional parameters:
       partition_prog - the program used to partion the data (Copy_split)
       collect_prog - the program used to partion the data (Copy_join)

       SEE parent class for more parameters

    """

    SEP.pf_base.parfile.__init__(self,**kw)
    self.type="COPY"
    self.isect=-1;
    self.partition_prog=self._option("partition_prog","%s/Copy_split"%SEP.paths.sepbindir)
    self.collect_prog=self._option("collect_prog","%s/Copy_join"%SEP.paths.sepbindir)

  def clone(self,**kw):
    """Clone a parallel file object (copy)"""
    kout=self.kw
    for key,val in kw.items(): kout[key]=val
    if not kw.has_key("space_only") and kout.has_key("space_only"):  
      del kout["space_only"]
    a=parfile(**kout)
    return a

  def _section_name(self,key,mlabel):
     """Get the file name if we have distributed a file

        key   - the section name
        mlabel- the machine label

        returns section_name,section_name if we have a new section
        returns section_name,None if the section alread exists


     """
     name=self._form_sect_name(key,SEP.mach.mach_from_label(mlabel))
     for file in self.get_stat_elem(2).values():
       if file==name: return name,None
     return name,name

  def _form_sect_name(self,key,mach):
     """Get the section name given the key and machinename"""
     self.isect+=1
     return "%s%s_par_DFF_%s"%(self._datapath(mach),os.path.basename(self.file_name),mach)

  def _collect_pars(self,keys):
    """Return the command line parameters for collecting a dataset

       key_mach a dictionary key:machine
       key_sect a dictionary key:section name

       returns a list that is command line parameters to collect the dataset

    """
    """Return the command line arguments when sharing a file"""
    pars=[self.collect_prog,"combo=%s"%self.file_name]
    if self.add_to: pars.append("add=1")
    mach_ithread={}
    sect_list=[]
    machs_list=[]
    nsect=0
    nmach=0
    machs_list=[]
    mach_key_back={}
    for key in keys:
      mach=SEP.mach.mach_from_label(self.get_key_status_elem(key,1))
      if machs_list.count(mach)==0: machs_list.append(mach)
      mach_key_back[mach]=key
    mach_sort=SEP.mach.mach.mach_order(machs_list)
    for  mach in mach_sort:
      key=mach_key_back[mach]
      sect=self.get_key_status_elem(key,2)
      if not mach_ithread.has_key(mach):
        nmach=nmach+1
        mach_ithread[mach]=mach_sort.index(mach)+1
        pars.append("in_tag_sect%d=%s"%(nsect,sect))
        pars.append("in_sect_thread%d=%s"%(nsect,mach_ithread[mach]))
        nsect=nsect+1
    #grab the first section 
    pars.append("in_nsect=%d"%(nsect))
    first=SEP.sepfile.sep_file(remote="%s:%s"%(mach,sect))
    nbytes=first.size()*nmach*8
    return pars,mach_sort,nbytes

  def input_size(self):
    """Return the size of the dataset"""
    return SEP.sepfile.sep_file(self.file_name).size()*8

  def axis(self,iaxis):
    """Return an axis (n,o,d,label,unit) in the dataset"""
    if self.space_only: SEP.util.err("Request for axis when dataset hasn't been set up %s"%self.file_name)
    if self.usage=="INPUT": return SEP.sepfile.sep_file(self.file_name).axis(iaxis)
    return self.sect_sepfile(self.return_keys()[0]).axis(iaxis)

  def _partition_pars(self,key_mach,key_sect):
    """Return the command line parameters for distributing a dataset

       key_mach a dictionary key:machine
       key_sect a dictionary key:section name

       returns a list that is command line parameters to partition the dataset

    """
    pars=[self.partition_prog,"combo=%s"%self.file_name]
    i=0;
    machs_in=[]
    mach_key_map={}
    for key,mach in key_mach.items():
      machs_in.append(key_mach[key])
      mach_key_map[mach]=key
    mach_sort=SEP.mach.mach.mach_order(machs_in)
    i=0; 
    for mach in mach_sort:
      key=mach_key_map[mach]
      pars.append("out_tag_sect%d=%s"%(i,key_sect[key]))
      pars.append("out_datapath_sect%d=%s"%(i,self._datapath(mach)))
      i=i+1
    nbytes=i*self.input_size()*8
    return pars,mach_sort,nbytes

