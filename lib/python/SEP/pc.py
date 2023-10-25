__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.3.18"
import SEP.util
                                                                                

remote=None

def set_communicator(com):
  SEP.pc.remote=com

def run_job(pars,machs,log,nbytes):
  if not remote: SEP.util.err("Parallel communicator has not been set")
  return remote.run_job(pars,machs,log,nbytes)

def clean():
  if not remote: SEP.util.err("Parallel communicator has not been set")
  return remote.clean()

def return_communicator():
  return remote
