import types,string,re
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.3.18"
                                                                                

shell="rsh -n"
cp="rcp"


def remote_com(mach,command):
  if type(command)==types.ListType: command=string.join(command)
  command=re.sub("'",'"',command)
  return '%s %s "%s"'%(shell,mach,command)

def cp_from(mach,file_in,file_out):
  return '%s %s:%s %s'%(cp,mach,file_in,file_out)

def cp_to(mach,file_in,file_out):
  return '%s %s %s:%s'%(cp,file_in,mach,file_out)


