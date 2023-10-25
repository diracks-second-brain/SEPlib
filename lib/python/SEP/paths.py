import os
"""A module for commonly need SEP paths"""
__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2004.12.2"
                                                                                


if os.environ.has_key("SEP"):
  sepdir=os.environ["SEP"]
else: sepdir="/usr/local/SEP"
if os.environ.has_key("MPI"):
  mpibindir="%s/bin/"%os.environ["MPI"]
else: mpibindir=""
sepbindir=sepdir+os.sep+"/bin"
