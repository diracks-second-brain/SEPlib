import re
import commands
import os
import string


def local_view(dir):
  resdir="Fig"
  how={}
  if os.path.exists("%s/view_rules"%dir):
    try:
      f=open("%s/view_rules"%dir)
      lines=f.readlines()
    except:
      lines=[]
    srch=re.compile("(\S+):(.+)")
    for line in lines:
      line=line.rstrip()
      match=srch.search(line)
      if match:
        how[match.group(1)]=match.group(2)
  if how.has_key("RESDIR"): resdir=how["RESDIR"]
  return resdir,how

def display(dir,tag):
  resdir,how=local_view(dir)
  if not how.has_key(tag):
    if os.path.exists("%s/%s/%s.T"%(dir,resdir,tag)):  
      com="Sep_cube %s/%s/%s.T "%(dir,resdir,tag)
    elif os.path.exists("%s/%s/%s.v3"%(dir,resdir,tag)):  
      com="tube %s/%s/%s.v3 pixmaps=y"%(dir,resdir,tag)
    elif os.path.exists("%s/%s/%s.v"%(dir,resdir,tag)):  
      com="tube %s/%s/%s.v pixmaps=y"%(dir,resdir,tag)
    elif os.path.exists("%s/%s/%s.pdf"%(dir,resdir,tag)):  
      com="acroread %s/%s/%s.pdf"%(dir,resdir,tag)
  else:
    com=how[tag]
    com=string.replace(com,"PATH",dir)
    com=string.replace(com,"RESDIR",resdir)
    print  com
  commands.getstatusoutput(com)

  


