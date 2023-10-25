import SEP.sep_socket
import SEP.sepfile
import os,sys,types,commands,string,re
import SEP.util
import SEP.datapath
import SEP.paths
import SEP.pf_base
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.12.22"
                                                                                
class parfile(SEP.pf_base.parfile):
  """A parallel file that is broken up along one or more axis"""

  def __init__(self,**kw):
    """Initialze a distributed file
    
       Required parameters:

       dff_axis - the axis/axes that the data is distributed along
       nblock   - the number of sections along each the dataset is broken into

       Optional parameters:
       partition_prog - the program used to partion the data (Patch_split)
       collect_prog - the program used to partion the data (Patch_join)

       SEE parent class for more parameters

    """

    SEP.pf_base.parfile.__init__(self,**kw)
    self.type="DISTRIBUTE"
    
    self.set_dff_axis(self._option("dff_axis",err=1))
    self.set_nblock(self._option("nblock",err=1))
    self.set_noverlap(None)

    if len(self.dff_axis)==99 and int(self.dff_axis[0])==1:
      combo="%s/Join_first"%SEP.paths.sepbindir
      part="%s/Split_first"%SEP.paths.sepbindir
    else:
      part="%s/Patch_split"%SEP.paths.sepbindir
      combo="%s/Patch_join"%SEP.paths.sepbindir
    self.partition_prog=self._option("partition_prog",part)
    self.collect_prog=self._option("collect_prog",combo)


  def done(self,key):
    """A job has finished """
    if self.usage=="INPUT": 
      self._remove_parts([key]);
      self.update_status(key,["done","none","none"])
    else: SEP.pf_base.parfile.done(self,key)


  def clone(self,**kw):
    """Clone a distributed file object"""
    kout={}
    for a,b in self.kw.items(): kout[a]=b
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
     name= self.get_key_status_elem(key,2)
     if name and name!="none": 
       machl=self.get_key_status_elem(key,1)
       if machl and machl!="none":
         mach_old=SEP.mach.mach_from_label(self.get_key_status_elem(key,1))
         mach_new=SEP.mach.mach_from_label(mlabel)
         if mach_old == mach_new: return name,None
     name=self._form_sect_name(key,SEP.mach.mach_from_label(mlabel))
     return name,name 

  def _form_sect_name(self,key,mach):
     """Get the section name given the key and machinename"""
     return "%s%s_par_DFF_%s"%(self._datapath(mach),os.path.basename(self.file_name),key)

  def _distributed_pars(self,pars,mach_ithread,machs):
    """Add parameters if the file is distributed"""
    pars.append("input_begin=1")
    return pars,mach_ithread,machs
      

  def input_size(self):
    return SEP.sepfile.sep_file(self.file_name).size()

  def _partition_pars(self,key_mach,key_sect):
    """Return the command line parameters for distributing a dataset

       key_mach a dictionary key:machine
       key_sect a dictionary key:section name

       returns a list that is command line parameters to partition the dataset

    """
    pars=[self.partition_prog,"combo=%s"%self.file_name]
    machs=[]; mach_distrib={}
    pars,mach_distrib,machs=self._distributed_pars(pars,mach_distrib,machs)
    pars.append("out_dff_axis=%s"%string.join(self.dff_axis,","))
    pars.append("out_noverlap=%s"%string.join(self.noverlap,","))
    pars.append("out_nblock=%s"%string.join(self.nblock,","))
    i=0;
    imach=len(machs)
    mach_thread={}

    machs_list=[]
    #ORDER THE MACHINESW FOR MAXIMUM EFFICIENCY THROUGH NETWORK
    for key in key_mach.keys():
      mach=SEP.mach.mach_from_label(self.get_key_status_elem(key,1))
      if not mach_thread.has_key(mach) and machs_list.count(mach)==0:
         machs_list.append(mach)
    mach_sort=SEP.mach.mach.mach_order(machs_list)
    for m in mach_sort: machs.append(m)
    for key,mach in key_mach.items():
      pars.append("out_tag_sect%d=%s"%(i,key_sect[key]))
      pars.append("out_datapath_sect%d=%s"%(i,self._datapath(mach)))
      if not mach_thread.has_key(mach):
        imach=imach+1;
        mach_thread[mach]=mach_sort.index(mach)+1;
      pars.append("out_sect_thread%d=%d"%(i,mach_thread[mach]))
      pars.append("out_isect_sect%d=%s"%(i,self.isect_dict[key]))
      i=i+1
    nbytes=i*self.input_size()*8
    pars.append("nsect=%d"%i)
    pars.append("out_nsect=%d"%i)
    for i in self.nblock: nbytes=nbytes/int(i)
    return pars,machs,nbytes

  def set_dff_axis(self,dff_axis):
    self.kw["dff_axis"]=dff_axis
    if type(dff_axis)!=types.ListType: dff_axis=[dff_axis]
    self.dff_axis=[]
    for i in dff_axis: self.dff_axis.append(str(i))
    self.add_file_param("dff_axis",dff_axis)

  def set_nblock(self,nblock):
    if type(nblock)!=types.ListType: nblock=[nblock]
    self.nblock=[]
    for i in nblock: self.nblock.append(str(i))
    self.add_file_param("nblock",nblock)
    if len(self.nblock) != len(self.dff_axis):
      SEP.util.err("Length of nblock and dff_axis not the same for %s"%self.file_name)

  def set_noverlap(self,noverlap):
    if not noverlap: 
      noverlap=[]
      for i in self.dff_axis: noverlap.append(0)
    if type(noverlap)!=types.ListType: noverlap=[noverlap]
    self.noverlap=[] 
    for i in noverlap: self.noverlap.append(str(0))
    self.add_file_param("noverlap",noverlap)
    if len(self.noverlap) != len(self.dff_axis):
      SEP.util.err("Length of noverlap and dff_axis not the same for %s"%self.file_name)

  def _collect_pars(self,keys):
    """Return the command line parameters for collecting a dataset

       key_mach a dictionary key:machine
       key_sect a dictionary key:section name

       returns a list that is command line parameters to collect the dataset

    """
    pars=[]
    pars.extend([self.collect_prog,"combo=%s"%self.file_name])
    pars,machs,mach_ithread=self.add_pars_all_sections(keys,pars)
    if self.add_to: pars.append("add=1")
    nbytes= self.sect_sepfile(self.return_keys()[0]).size()*8
    return pars,machs,nbytes

  def sepfile_from_sects(self):
    """Return the sepfile related to the entire dataset"""
    all={}
    for key in self.return_keys(): 
      all[key]=self.sect_sepfile(key)
    outs=SEP.sepfile.sep_file(name="S",sepfile=all["0"])
    for iax in self.dff_axis:
      nout=0
      for a in all.values(): 
        nout+=a.axis(iax)[0]
      for i in range(len(self.dff_axis)):
        ib=self.dff_axis[i]
        if ib != iax: nout=nout/self.nblock[i]
      outs.history.add_param("n%d"%int(iax),nout)
    return outs
    
      

#  def axis(self,iaxis):
#    """Return an axis (n,o,d,label,unit) in the dataset"""
#    if self.space_only: SEP.util.err("Request for axis when dataset hasn't been set up %s"%self.file_name)
#    if self.usage=="INPUT": return SEP.sepfile.sep_file(self.file_name).axis(iaxis)
#    if count(self.dff_axis(iaxis))==0: 
#      return self.sect_sepfile(self.return_keys()[0]).axis(iaxis)
#    else: 
#      n,o,d,label,unit=first=self.sect_sepfile("1").axis(iaxis)
#      n=0
#      for key in self.return_keys(): n+=self.sect_sepfile(key).axis(iaxis)[0]
#      for iax in self.nblock:
#        if iax != iaxis: n=n/self.nblock()
#      return n,o,d,label,unit
#       



  def add_pars_all_sections(self,keys,pars,prefix="in_",machs=[],mach_ithread={}):
    """Return the parameters for the entire dataset
       Useful when combining a dataset or using a distributed dataset """
    pars.append("%sdff_axis=%s"%(prefix,string.join(self.dff_axis,",")))
    pars.append("%snblock=%s"%(prefix,string.join(self.nblock,",")))
    pars.append("%snoverlap=%s"%(prefix,string.join(self.noverlap,",")))
    nsect=0
    mach_ithread={} #thsi shouldn't bee here but their appears to be a bug in python
    nmach=len(mach_ithread.keys())
    machs_list=[]
    for key in keys:
      mach=SEP.mach.mach_from_label(self.get_key_status_elem(key,1))
      if not mach_ithread.has_key(mach) and machs_list.count(mach)==0:
         machs_list.append(mach)
    mach_sort=SEP.mach.mach.mach_order(machs_list)
    machs=[]
    for m in mach_sort: machs.append(m)
    nmach_base=nmach
    for  key in keys:
      if "none"==self.get_key_status_elem(key,1):
        SEP.util.err("Internal error: collecting=%s key=%s "%(self.file_name,key))
      mach=SEP.mach.mach_from_label(self.get_key_status_elem(key,1))
      sect=self.get_key_status_elem(key,2)
      if not mach_ithread.has_key(mach):
        nmach=nmach+1
        mach_ithread[mach]=mach_sort.index(mach)+nmach_base+1
      pars.append("%stag_sect%d=%s"%(prefix,nsect,sect))
      pars.append("%ssect_thread%d=%s"%(prefix,nsect,mach_ithread[mach]))
      pars.append("%sisect_sect%d=%s"%(prefix,nsect,self.isect_dict[key]))
      pars.append("%sdatapath_sect%d=%s"%(prefix,nsect,self._datapath(mach)))
      nsect=nsect+1
    pars.append("%snsect=%d"%(prefix,nsect))
    return pars,machs,mach_ithread
