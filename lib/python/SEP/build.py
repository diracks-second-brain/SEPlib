import commands,re,string,sys,os
import SEP.make
import SEP.util

def nobuild_ps_pdf(enviro,fig,verbose):
  """Create pdf file from a postscript file"""
  ps="%s/%s.ps"%(enviro.val("RESDIR"),fig)
  pdf="%s/%s.pdf"%(enviro.val("RESDIR"),fig)
  sh="%s/%s.sh"%(enviro.val("RESDIR"),fig)
  if not os.path.isfile(ps):
    return  "%s does not exist "%ps,1
  output,stat=SEP.make.command( "-s -n %s"%pdf)
  if stat!=0: return "%s: problem with the rules for building pdf"%pdf,1
  os.system("gmake %s"%pdf)
  if not os.path.isfile(sh):
    os.system('/bin/echo "cd %s; gmake SEMINAR=yes %s.view" > %s'%(os.curdir,fig,sh))
    os.system("chmod a+x %s"%sh)
  return " ",0

  


def build_ps_pdf(enviro,fig,verb=1):
  """Build a PDF file rebuilding the postscript file if not up to date"""
  print "IN BUILD PS PDF",fig
  #this would become automatic under scons
  output,stat=SEP.make.command(" -s -n %s.build"%fig)
  if stat!=0: return "%s: Problem with build rules :  %s"%(fig,output),1
  if not verb: extra=" >&/dev/null"
  else: extra="" 
  stat=os.system("gmake  %s.build %s"%(fig,extra))
  if stat!=0: return "%s does not build properly",1
  return nobuild_ps_pdf(enviro,fig,verb)
  

def build_pdf(enviro,fig,buildcr=1,verbose=1):
  """Function to build pdf file"""
  if enviro.val("RESULTS").count(fig)==0: return "%s not listed in results "%fig,1
  if enviro.val("RESULTSER").count(fig)==1: 
    return build_ps_pdf(enviro,fig,verbose)
  if enviro.val("RESULTSCR").count(fig)==1:
    if buildcr: return build_ps_pdf(enviro,fig,verbose)
    else: return nobuild_ps_pdf(enviro,fig,verbose)
  return nobuild_ps_pdf(enviro,fig,verbose)

