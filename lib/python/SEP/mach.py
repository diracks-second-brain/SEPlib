import re
import SEP.util
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.4.8"
                                                                                

mach=None
srch=re.compile("(.+)-\d+$")

def set_machine_method(com):
  mach=com

def return_mach():
  return mach

def mach_from_label(label):
  a=srch.search(label)
  if not a: SEP.util.err("Can't interpret %s "%label)
  return a.group(1)
  
