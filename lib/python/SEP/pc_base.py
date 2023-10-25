import SEP.log
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.3.18"
                                                                                
class communicator:
  """The base class to handling parallel operations"""
  def __init__(self,startup=40,speed=12,logfiletag="remote"):
      """Initialze the mpi object
 
          startup - the maximum time it takes to start a job on the node
          speed   - the minimum speed in transfreing (in megabytes per second)

      """
      self.startup=startup
      self.speed=speed
      self.logfiletag=logfiletag

  def clean(self):
    """Clean up from a parallel job"""
       
  def run_job(self,pars,nodes,nbytes):
     """Run a parallel job

         pars  - the command line arguments
         nodes - the nodes to run
         nbytes- the number of bytes we are sending
     """
     SEP.util.err("Must override how to run a parallel job")

  def wait_time(self,bytes,nmach):
    """The maximum amount it should take to run a given job

        bytes - the number of bytes we are transfering
        nmach - the number of machines we are running on

    """
    tm=self.startup*nmach #startup time from processes
    tm+=bytes/self.speed*100000/1000000 #passing time 
    return tm
