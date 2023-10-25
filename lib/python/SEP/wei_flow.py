import SEP.opt_none
import SEP.lloyd
import SEP.util
import SEP.sepfile
import SEP.wei_off_mig
import SEP.wei_off_model
import SEP.wei_ang_mig

__author__ = "Robert G. Clapp"
__email__ = "bob@sep.stanford.edu"
__version = "2005.3.13"
                                                                                


class wtransp_interp_model(SEP.opt_prog.options):
  """A transpose/interpolation operation that is useful for wei"""
  def __init__(self,main_par):
    SEP.opt_prog.options.__init__(self,"WEI.TI")
    self.add_doc_params()
    self.main=main_par
  def add_doc_params(self):
    """Parameters for the transp wei needs"""
    self.add_doc_param("maxsize",100,"Approximate memory to use")
    self.add_doc_param("interp_type",1,"Interpolation")
  def build_check_params(self):
    """Build the commands for the program"""
    vel=self.main.param("velocity")
    image=SEP.sepfile.sep_file(self.main.param("R"))
    nz,oz,dz,label,unit=image.axis(5)
    vout="mig_%s"%vel
    self.add_param("vel_mig",vout)
    self.command="<%s %s/Interp maxsize=%s type=%s  "%(vel,
      SEP.paths.sepbindir,self.param("maxsize"),self.param("interp_type"))
    self.command="%s n1out=%d o1out=%f d1out=%f |%s/Transp"%(self.command,
      nz,oz,dz,SEP.paths.sepbindir)
    self.command="%s reshape=1,3 maxsize=%s >%s plane=12 "%(self.command,
      self.param("maxsize",vout),vout)
  def prep_run(self,add_args=None):
    """Run the Interp|Transp program"""
    SEP.spawn.run_wait(self.command,verb=self.param("verb",error=None)) 



class wtransp_interp(SEP.opt_prog.options):
  """A transpose/interpolation operation that is useful for wei"""
  def __init__(self,main_par):
    SEP.opt_prog.options.__init__(self,"WEI.TI")
    self.add_doc_params()
    self.main=main_par
  def add_doc_params(self):
    """Parameters for the transp wei needs"""
    self.add_doc_param("maxsize",100,"Approximate memory to use")
    self.add_doc_param("interp_type",1,"Interpolation")
  def build_check_params(self):
    """Build the commands for the program"""
    vel=self.main.param("velocity")
    oz=float(self.main.param("oz"))
    dz=float(self.main.param("dz"))
    nz=int(self.main.param("nz"))
    vout="mig_%s"%vel
    self.add_param("vel_mig",vout)
    self.command="<%s %s/Interp maxsize=%s type=%s  "%(vel,
      SEP.paths.sepbindir,self.param("maxsize"),self.param("interp_type"))
    self.command="%s n1out=%d o1out=%f d1out=%f |%s/Transp"%(self.command,
      nz,oz,dz,SEP.paths.sepbindir)
    self.command="%s reshape=1,3 maxsize=%s >%s plane=12 "%(self.command,
      self.param("maxsize",vout),vout)
  def prep_run(self,add_args=None):
    """Run the Interp|Transp program"""
    SEP.spawn.run_wait(self.command,verb=self.param("verb",error=None)) 


class lloyd_wei(SEP.lloyd.lloyd):
  """Lloyd for WEI"""
  def __init__(self,main_args):
    """Initialze the lloyd operator WEI migration"""
    self.main=main_args
    SEP.lloyd.lloyd.__init__(self,"WEI.LLOYD")
  def add_doc_params(self):
    """Add the documentation for Lloyd"""
    SEP.lloyd.lloyd.add_doc_params(self)
    self.del_doc_param("stdin")
    self.del_doc_param("stdout")
    self.del_doc_param("vel_map")
  def build_check_params(self):
    """Build commands for Lloyd"""
    vel="mig_%s"%self.main.param("velocity")
    schoice="ref_%s"%vel
    self.add_param("stdin",vel)
    self.add_param("stdout",schoice)
    self.add_param("schoice",schoice)
    SEP.lloyd.lloyd.build_check_params(self)

class off_model(SEP.wei_off_model.offset_modeling):
  def __init__(self,main_args):
     SEP.wei_off_model.offset_modeling.__init__(self,"WEI.MODEL")
     self.main=main_args
  def add_doc_params(self):
     SEP.wei_off_model.offset_modeling.add_doc_params(self)
     self.del_axes_params(["z_"])
     self.del_doc_param("program")
     self.del_doc_param("S")
     self.del_doc_param("schoice")
     self.del_doc_param("D")
     self.del_doc_param("R")
  def build_check_params(self):
    """Build the commands for the program"""
    self.add_param("program",self.main.param("program"))
    vel=self.main.param("velocity")
    ow=float(self.main.param("ow"))
    dw=float(self.main.param("dw"))
    nw=int(self.main.param("nw"))
    self.add_param("aw__n",nw)
    self.add_param("aw__o",ow)
    self.add_param("aw__d",dw)
    self.velocity=self.main.param("velocity")
    self.add_param("schoice","ref_mig_%s"%self.velocity)
    self.add_param("S","mig_%s"%self.velocity)
    self.add_param("R",self.main.param("R"))
    self.add_param("D",self.main.param("D"))
    SEP.wei_off_model.offset_modeling.build_check_params(self)


class off_mig(SEP.wei_off_mig.migration):
  def __init__(self,main_args):
     SEP.wei_off_mig.migration.__init__(self,"WEI.MIG")
     self.main=main_args
  def add_doc_params(self):
     SEP.wei_off_mig.migration.add_doc_params(self)
     self.del_axes_params(["z_"])
     self.del_doc_param("program")
     self.del_doc_param("S")
     self.del_doc_param("schoice")
     self.del_doc_param("D")
     self.del_doc_param("R")
  def build_check_params(self):
    """Build the commands for the program"""
    self.add_param("program",self.main.param("program"))
    vel=self.main.param("velocity")
    oz=float(self.main.param("oz"))
    dz=float(self.main.param("dz"))
    nz=int(self.main.param("nz"))
    self.add_param("az__n",nz)
    self.add_param("az__o",oz)
    self.add_param("az__d",dz)
    self.velocity=self.main.param("velocity")
    self.add_param("schoice","ref_mig_%s"%self.velocity)
    self.add_param("S","mig_%s"%self.velocity)
    self.add_param("R",self.main.param("R"))
    self.add_param("D",self.main.param("D"))
    SEP.wei_off_mig.migration.build_check_params(self)



    
class ang_mig(SEP.wei_ang_mig.migration):
  def __init__(self,main_args):
     SEP.wei_ang_mig.migration.__init__(self,"WEI.MIG")
     self.main=main_args
  def add_doc_params(self):
     SEP.wei_ang_mig.migration.add_doc_params(self)
     self.del_axes_params(["z_"])
     self.del_doc_param("program")
     self.del_doc_param("S")
     self.del_doc_param("schoice")
     self.del_doc_param("D")
     self.del_doc_param("R")
  def build_check_params(self):
    """Build the commands for the program"""
    self.add_param("program",self.main.param("program"))
    vel=self.main.param("velocity")
    oz=float(self.main.param("oz"))
    dz=float(self.main.param("dz"))
    nz=int(self.main.param("nz"))
    self.add_param("az__n",nz)
    self.add_param("az__o",oz)
    self.add_param("az__d",dz)
    self.velocity=self.main.param("velocity")
    self.add_param("schoice","ref_mig_%s"%self.velocity)
    self.add_param("S","mig_%s"%self.velocity)
    self.add_param("R",self.main.param("R"))
    self.add_param("D",self.main.param("D"))
    SEP.wei_ang_mig.migration.build_check_params(self)
    
class model_main(SEP.opt_none.options):
  """The migration """
  def __init__(self):
    SEP.opt_none.options.__init__(self,"WEI.MAIN")
    self.add_doc_params()
  def add_doc_params(self):
    """Parameters for the transp wei needs"""
    self.add_doc_param("nw",doc="Number of frequencies")
    self.add_doc_param("ow",doc="First w")
    self.add_doc_param("D",doc="Data (mx,my,hx,hy,w)")
    self.add_doc_param("R",doc="Image (mx,my,hx/px,hy/py,z")
    self.add_doc_param("dw",doc="Sampling in w")
    self.add_doc_param("velocity",doc="Velocity Model")
    self.add_doc_param("restart",doc="Whether or not to restart",required=None)
    self.add_doc_param("program",doc="WEI program to use")
    print "IN HERE 1"

class mig_main(SEP.opt_none.options):
  """The migration """
  def __init__(self):
    SEP.opt_none.options.__init__(self,"WEI.MAIN")
    self.add_doc_params()
  def add_doc_params(self):
    """Parameters for the transp wei needs"""
    self.add_doc_param("nz",doc="Number of depth steps")
    self.add_doc_param("oz",doc="First depth")
    self.add_doc_param("D",doc="Data (mx,my,hx,hy,w)")
    self.add_doc_param("R",doc="Image (mx,my,hx/px,hy/py,z")
    self.add_doc_param("dz",doc="Sampling in depth")
    self.add_doc_param("velocity",doc="Velocity Model")
    self.add_doc_param("restart",doc="Whether or not to restart",required=None)
    self.add_doc_param("program",doc="WEI program to use")


