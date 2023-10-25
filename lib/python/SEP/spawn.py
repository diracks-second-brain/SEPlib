import os,sys,time,pwd,string,types,popen2
import SEP.util
import SEP.datapath
import commands
import SEP.log
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2004.7.30"
                                                                                
print_all=None
com_file=None
write_all=1



def run_wait(cmd,oper_len=9999999.,verb=None,logfile=None,sleep_interv=1,error=1,quiet=None,ntrys=1):
    """Run a command

        cmd            can be a string or a list
        oper_len       is the length of time before killing the given process
        verb           write command before running it
        logfile        write stderr and stdout to given file
        sleep_interv   sleep interval when waiting for a job to finish
        error          give an error if the command fails
        quiet          don't report stderr,stdout if it fails
        ntrys          Number of times to try the operation

    """
    if type(cmd)==types.ListType: cmd=string.join(cmd)

    if logfile: loguse=logfile
    else:       loguse="%sxyza.%s"%(SEP.datapath.datapath(),os.getpid())
    cm2=(' %s stderr=%s'%(cmd,loguse))
    cmd=('(%s)>&%s'%(cmd,loguse))
    if verb or print_all: SEP.util.msg("time:%s cmd=%s"%(str(oper_len),cmd))
    itry=0
    if write_all:
      if not SEP.spawn.com_file:
        SEP.spawn.com_file="%s/%s"%(SEP.log.logging.return_log_dir(),"all_coms")
        stat,out=commands.getstatusoutput("rm %s"%SEP.spawn.com_file)
      
      stat,out=commands.getstatusoutput("echo %s >> %s "%(cm2,SEP.spawn.com_file))
    while itry < ntrys:
      itry=itry+1
      tmpg=SEP.util.temp_file("good")
      tmpb=SEP.util.temp_file("bad")
      pid=os.fork() 
      mytime=time.time()+oper_len
      if pid: #master thread
        stime=.02
        while  time.time() < mytime:
          if  os.path.isfile(tmpg) or os.path.isfile(tmpb): break
          time.sleep(stime)
          if stime < sleep_interv: stime=stime+.02
      else:
        stat,out=commands.getstatusoutput(cmd)
        if stat==0: stat,out=commands.getstatusoutput("touch %s"%tmpg)
        else:  stat,out=commands.getstatusoutput("touch %s"%tmpb)
        sys.exit(0)
      if not os.path.isfile(tmpg):
        os.kill(pid,9)
        if loguse:
          try: f=open(loguse)
          except:  return 1
          line=f.readline()
          while line:
            if not quiet: SEP.util.msg(line.rstrip())
            line=f.readline()
          f.close()
        if error and itry==ntrys: 
          SEP.util.err("ERR:Killed or died pid=%d\n%s\n"%(pid,cmd))
        elif not quiet: SEP.util.msg("WARN:Killed or died pid=%d\n%s\n"%(pid,cmd))
        if logfile: stat,out=commands.getstatusoutput("rm %s %s"%(tmpg,tmpb))
        else: stat,out=commands.getstatusoutput("rm %s %s %s"%(tmpg,tmpb,loguse))
        if itry== ntrys :return 1 
        print "trying again"
      else: 
        os.waitpid(pid,0)
        stat,out=commands.getstatusoutput("rm %s %s"%(tmpg,tmpb))
        return None

