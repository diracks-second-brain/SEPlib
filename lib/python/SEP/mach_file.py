import time,re,os,commands,string,pwd
import SEP.spawn
import SEP.util
import SEP.datapath
import SEP.stat_sep
import SEP.mach_base
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.3.18"
                                                                                

class machine(SEP.mach_base.machine):
  """class containing list of potential machines to run on"""
                                                                                
  def __init__(self,mfile="mfile",nmax=2000):
    """Initialize the machine database

       mfile - the machine file ("mfile")
       njobs - the maximum number of jobs to run simutaneously

    """
   
    self.mfile=mfile
    SEP.mach_base.machine.__init__(self,nmax)

  def all_label_mach(self):
    """Get the list of machines from a file"""
    machinefile=self.mfile
    try: f = open (machinefile)
    except:  SEP.util.err("Trouble opening file:"+machinefile)
    lines=f.readlines()
    nocommentS=re.compile('^(.+)\#')    #remove commented material
    machnS=re.compile('^(\S+):(\S+)')    #remove commented material
    machS=re.compile('^(\S+)')    #remove commented material
    machname={}
    nmach={}
    iorder=0
    for line in lines:
       #remove the commented material
       nocomment=nocommentS.search(line.strip())
       if nocomment: do=nocomment[0]
       else: do=line.strip()
       ncount=0;
       machn=machnS.search(do)
       mach=machS.search(do)
       if machn:
         ncount=machn.group(2)
         mach=machn.group(1)
       if mach:
         ncount=1
         mach=mach.group(1)
       if not self.machine_order.has_key(mach):
         self.machine_order[mach]=iorder
         iorder=iorder+1
       if nmach.has_key(mach): n=nmach[mach]
       else: n=0
       for i in range(ncount):
         name=mach+"-"+str(n)
         machname[name]=mach
         n+=1
       nmach[mach]=n
    f.close()
    return machname

