                                                                                    
def dff_part(ipart,nblock,n,o,d):
  nsmall=int(n/nblock)
  nextra=n-nblock*nsmall
  ifirst=ipart*nsmall
  if ipart >= nextra:
    n=nsmall; o=o+d*(ifirst+nextra)
  else:
    n=nsmall+1; o=o+d*(ifirst+ipart)
  return n,o,d
                                                                                    
def clean_vec_keywords(kw,clean):
  kout=cp_kw(kw)
  for key in clean:
    if kout.has_key(key): del kout[key]
  return kout
                                                                                    
def cp_kw(kw):
  kout={}
  for k,v in kw.items(): kout[k]=v
  return kout
                                                                                    
def combine_keywords(kw1,kw2):
  kout=cp_kw(kw1)
  for key,val in kw2.items(): kout[key]=val
  return kout
                                                                                    
