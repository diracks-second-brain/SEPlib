import SEP.util
import SEP.make
import os


class dir_command:
  """The results of executing a command in a directory"""
  def __init__(self,dir_parent,my_dir,command):
    self.command=command




class tour_command:
  def __init__(self,command,dirs):
    self.command=commmand
    self.dir_list=[]
    self.my_dir=os.curdir()
    for dir in dirs: self.dir_list.append(dir_command(self.my_dir,dir,command)



class tour:
  """Tour a series of directories running specified command"""
  def __init__(self,dir_list=[],verb=None):
    self.verb=verb
    self.results=[]
    if not dir_list:
      self.dirs=[]
      d=SEP,make.get_vals("PAPERS")
      for dir in d: self.dirs.append("../%s"%dir)

  def execute_make(self,command):
    """Execute a make command in the directories"""
    self.execute_shell("make %s"%command)

  def executre_shell(self,command):
    """Execute a shell command in the directories"""
    self.results.append(tour_command(command,self.dirs))
    

  def print_summary(self):
     
  def
