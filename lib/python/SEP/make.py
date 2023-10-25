import commands,re,string,sys,os
import SEP.util


def  command(com):
   """Get the results of a make request"""
   file=SEP.util.temp_file("MAKE")
   stat1,o= commands.getstatusoutput("make %s > %s "%(com,file))
   if stat1==0:
     f=open(file)
     line=string.join(f.readlines(),"\n")
     if line and line[-1] == '\n': line = line[:-1] 
     stat2,o= commands.getstatusoutput("rm %s "%(file))
     return line,0
   return o,1

def  get_val(key):
   """Get defined values in the makefile"""
   msg,stat=command("%s.varvalue"%key)
   if stat!=0: SEP.util.err("Trouble with makefile\n%s"%msg)
   return msg

def  get_vals(key):
   """Get defined values in the makefile in list form"""
   return get_val(key).split()

class environment:
  """A class containing the basic SEP environmental variables"""

  def __init__(self):
    self.enviro={} 
    single=["RESDIR"]
    multiple=["RESULTSER","RESULTSCR","RESULTSNR"]
    for i in single: self.enviro[i]=get_val(i)
    for i in multiple: self.enviro[i]=get_vals(i)
    self.enviro["RESULTS"]=get_vals("RESULTSER")
    self.enviro["RESULTS"].extend(self.enviro["RESULTSCR"])
    self.enviro["RESULTS"].extend(self.enviro["RESULTSNR"])

  def val(self,key):
    if self.enviro.has_key(key): return self.enviro[key]
    return " "

  def printit(self):
    """Print out the environmental variables"""
    for key,val in self.enviro.items(): print "%s=%s"%(key,val)


