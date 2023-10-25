import SEP.spawn

my_base={}


def store(name,opt):
  my_base[name]=opt  

def find(name):
   if not my_base.has_key(name): return None
   return my_base[name]
