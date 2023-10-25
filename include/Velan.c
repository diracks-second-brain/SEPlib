
 /*$

=head1 NAME

Velan - Velocity analysis
 
=head1 SYNOPSIS

Velan velopt=vel nv= dv= v0= nw= dw= w0= ng=all g0=0 dg=1 ntout=nt/10 dtout=(nt/ntout)*dt t0out=t0 tsmoo=2*dtout smute=2 < in.H > out.H  

=head1 INPUT PARAMETERS

=over 4

=item  velopt  - char*   

      vel - scan over velocity (nv,dv,v0 required)
      slow - scan over slowness (nw,dw,w0 required)

=item  nv      - int     

      number of velocities

=item  dv,v0   - float   

      sampling and first velocity

=item  nw      - int     

      number pf slownesses

=item  dw,w0   - float   

      sampling and first slownesses

=item  ng      - int     

      [all] number of gathers analyzed

=item  g0      - int     

      [0]   number of gathers to skip before beginning analysis

=item  dg      - int     

      [1]   increment between gathers analyzed

=item  ntout   - int     

      [nt/10]  time-axis of out

=item  dtout   - float   

      [(nt/ntout)*dt] dtout must be an integer multiple of dt

=item  t0out   - float   

      [t0]first time to analyze

=item  tsmoo   - float   

      [2*dtout]  length of temporal smoothing window

=item  smute   - float   

      [2] samples for which nmo stretch exceeds smute 
      are zeroed

=item  n1,n2,n3-int      

      standard seplib params

=item  o1,o2,o3-float     

      standard seplib params

=item  d1,d2,d3-float     

      standard seplib params

=back

=head1 OUTPUT PARAMETERS

=over 4


=item  n1 - integer    

      ntout  

=item  d1 - float      

      dtout  

=item  o1 - float      

      t0out   

=item  n2 - int        

      nv 

=item  d2 - float      

      dv 

=item  o2 - float      

      v0  

=item  n3 - int        

      ng midpoint axis

=item  d3 - float      

      (input d3)*dg     

=item  o3 - float      

      (input o3) + g0*(input d3)  

=back

=head1 DESCRIPTION

Velocity analysis of common-midpoint gathers  

=head1 SEE ALSO

L<Velan3d>

=head1 CATEGORY

B<seis/velan>

=cut

>*/

/*


 KEYWORDS    velocity-analysis nmo hyperbola cmp

 SEE ALSO
  NMO

 WARNING
  Will not accept input from pipes.

 WHERE
  ./cube/seis/Velan.c

*/
/*
Dave Hale, 6/7/83
-> seplib: Shuki, 8/1/83.
seplib standards: John T., 11-11-83
slowness option added: John T., 2-1-84
converted to 4.2 apex drive: S. Levin 2/21/84
nvo > 1 and ng > 1 bug fixed: Rick O. 2/28/84
apnice() subroutine added 4-14-84 by Lloyd
ng>1 and dg>1 bug fixed: Stewart A. Levin  6/19/85
Convex conversion  Stewart A. Levin  11/9/85
added signal handling for vdiv by zero  Stewart A. Levin 12/5/85
*/
/*
 * Keyword: velocity-analysis nmo hyperbola cmp : Nmo
 */

#define MY_SEP_DOC \
 sep_add_doc_line("NAME");\
 sep_add_doc_line("    Velan - Velocity analysis");\
 sep_add_doc_line("");\
 sep_add_doc_line("SYNOPSIS");\
 sep_add_doc_line("    Velan velopt=vel nv= dv= v0= nw= dw= w0= ng=all g0=0 dg=1 ntout=nt/10");\
 sep_add_doc_line("    dtout=(nt/ntout)*dt t0out=t0 tsmoo=2*dtout smute=2 < in.H > out.H");\
 sep_add_doc_line("");\
 sep_add_doc_line("INPUT PARAMETERS");\
 sep_add_doc_line("    velopt - char*");\
 sep_add_doc_line("              vel - scan over velocity (nv,dv,v0 required)");\
 sep_add_doc_line("              slow - scan over slowness (nw,dw,w0 required)");\
 sep_add_doc_line("");\
 sep_add_doc_line("    nv - int");\
 sep_add_doc_line("              number of velocities");\
 sep_add_doc_line("");\
 sep_add_doc_line("    dv,v0 - float");\
 sep_add_doc_line("              sampling and first velocity");\
 sep_add_doc_line("");\
 sep_add_doc_line("    nw - int");\
 sep_add_doc_line("              number pf slownesses");\
 sep_add_doc_line("");\
 sep_add_doc_line("    dw,w0 - float");\
 sep_add_doc_line("              sampling and first slownesses");\
 sep_add_doc_line("");\
 sep_add_doc_line("    ng - int");\
 sep_add_doc_line("              [all] number of gathers analyzed");\
 sep_add_doc_line("");\
 sep_add_doc_line("    g0 - int");\
 sep_add_doc_line("              [0]   number of gathers to skip before beginning analysis");\
 sep_add_doc_line("");\
 sep_add_doc_line("    dg - int");\
 sep_add_doc_line("              [1]   increment between gathers analyzed");\
 sep_add_doc_line("");\
 sep_add_doc_line("    ntout - int");\
 sep_add_doc_line("              [nt/10]  time-axis of out");\
 sep_add_doc_line("");\
 sep_add_doc_line("    dtout - float");\
 sep_add_doc_line("              [(nt/ntout)*dt] dtout must be an integer multiple of dt");\
 sep_add_doc_line("");\
 sep_add_doc_line("    t0out - float");\
 sep_add_doc_line("              [t0]first time to analyze");\
 sep_add_doc_line("");\
 sep_add_doc_line("    tsmoo - float");\
 sep_add_doc_line("              [2*dtout]  length of temporal smoothing window");\
 sep_add_doc_line("");\
 sep_add_doc_line("    smute - float");\
 sep_add_doc_line("              [2] samples for which nmo stretch exceeds smute ");\
 sep_add_doc_line("              are zeroed");\
 sep_add_doc_line("");\
 sep_add_doc_line("    n1,n2,n3-int");\
 sep_add_doc_line("              standard seplib params");\
 sep_add_doc_line("");\
 sep_add_doc_line("    o1,o2,o3-float");\
 sep_add_doc_line("              standard seplib params");\
 sep_add_doc_line("");\
 sep_add_doc_line("    d1,d2,d3-float");\
 sep_add_doc_line("              standard seplib params");\
 sep_add_doc_line("");\
 sep_add_doc_line("OUTPUT PARAMETERS");\
 sep_add_doc_line("    n1 - integer");\
 sep_add_doc_line("              ntout");\
 sep_add_doc_line("");\
 sep_add_doc_line("    d1 - float");\
 sep_add_doc_line("              dtout");\
 sep_add_doc_line("");\
 sep_add_doc_line("    o1 - float");\
 sep_add_doc_line("              t0out");\
 sep_add_doc_line("");\
 sep_add_doc_line("    n2 - int");\
 sep_add_doc_line("              nv");\
 sep_add_doc_line("");\
 sep_add_doc_line("    d2 - float");\
 sep_add_doc_line("              dv");\
 sep_add_doc_line("");\
 sep_add_doc_line("    o2 - float");\
 sep_add_doc_line("              v0");\
 sep_add_doc_line("");\
 sep_add_doc_line("    n3 - int");\
 sep_add_doc_line("              ng midpoint axis");\
 sep_add_doc_line("");\
 sep_add_doc_line("    d3 - float");\
 sep_add_doc_line("              (input d3)*dg");\
 sep_add_doc_line("");\
 sep_add_doc_line("    o3 - float");\
 sep_add_doc_line("              (input o3) + g0*(input d3)");\
 sep_add_doc_line("");\
 sep_add_doc_line("DESCRIPTION");\
 sep_add_doc_line("    Velocity analysis of common-midpoint gathers");\
 sep_add_doc_line("");\
 sep_add_doc_line("SEE ALSO");\
 sep_add_doc_line("    Velan3d");\
 sep_add_doc_line("");\
 sep_add_doc_line("CATEGORY");\
 sep_add_doc_line("    seis/velan");\
 sep_add_doc_line("");

#include <sep.main>
MAIN()
  {
  int ng,nf,nf0,nt,nv,nw,ig,dg,g0,jf,ivi,ivo,nvi,nvi1,nvitemp=10,nvo,iv,iv0;
  int astrt,ap,anum,aden,annz,affovv,amute,asclrs,aspace,aend;
  int icdp,i0out,idout,ntout;
  int esize,it,smooth;
  float dt,df,dv,v,sclrsa[11],sclrsb[3],fract;
  float v0,dtout,t0out,tsmoo,t0,f0,f,smute,sssm1,velocity;
  float w0,dw,w,tempr;
  float *input,*output,*num,*denom,*offslow,*vs,*ws;
  float numsum, densum,tau,time;
  int *count,*mutetime,begi,endi,i0;
  char velopt[20],verbose[10]; int vv;
  int ione=1,itwo=2,itemp,tuse;
  int i1,i2;

  sep_begin_prog();

  /* obtain parameters and open files */
  if(!fetch("esize","d",&esize)) esize=sizeof(float);
    putch("ntin","d",&nt);
  if(!fetch("n1","d",&nt)) if(!fetch("nt","d",&nt)) seperr("need nt\n");
    putch("ntin","d",&nt);
  if(!fetch("d1","f",&dt)) if(!fetch("dt","f",&dt)) seperr("need dt\n");
    putch("dtin","f",&dt);
  if(!fetch("o1","f",&t0)) if(!fetch("t0","f",&t0)) seperr("need t0\n");
    putch("t0in","f",&t0);
  if(!fetch("n2","d",&nf)) if(!fetch("nf","d",&nf)) seperr("need nf\n");
    putch("nfin","d",&nf);
  if(!fetch("n3","d",&ng)) if(!fetch("ng","d",&ng)) seperr("need ng\n");
    putch("ngin","d",&ng);
  if(!fetch("d2","f",&df)) if(!fetch("df","f",&df)) seperr("need df\n");
    putch("dfin","f",&df);
  if(!fetch("o2","f",&f0)) 
    if(!fetch("f0","f",&f0)) seperr("need f0\n");
  strcpy(velopt,"vel"); getch("velopt","s",velopt);
  if(velopt[0]=='v')
     {
     if(!getch("nv","d",&nv)) seperr("need nv\n");putch("nv= n2","d",&nv);
     if(!getch("dv","f",&dv)) seperr("need dv\n");putch("dv= d2","f",&dv);
     if(!getch("v0","f",&v0)) seperr("need v0\n");putch("v0= o2","f",&v0);
		 vs=(float*) alloc(sizeof(float)*nv);
     if(v0 == 0.) seperr("v0 can't be 0\n");
     for(iv=0;iv<nv;iv++) vs[iv]=v0+iv*dv;
     }

  if(velopt[0]=='s')
     {
     if(!getch("nw","d",&nw)) seperr("need nw\n");putch("nw= n2","d",&nw);
     if(!getch("dw","f",&dw)) seperr("need dw\n");putch("dw= d2","f",&dw);
     if(!getch("w0","f",&w0)) seperr("need w0\n");putch("w0= o2","f",&w0);
     nv=nw; /* nv is used for space considerations and formation of nvi,nvo */
     }

  dg = 1;      getch("dg","d",&dg); putch("dg= d3","d",&dg);
  g0 = 0;      getch("g0","d",&g0); putch("g0= o3","d",&g0);

  ntout = nt/10;  getch("ntout","d",&ntout);putch("n1","d",&ntout);
  dtout = (nt/ntout)*dt;getch("dtout","f",&dtout);putch("d1","f",&dtout);
  t0out = t0;  getch("t0out","f",&t0out); putch("o1","f",&t0out);
  tsmoo = 2*dtout;getch("tsmoo","f",&tsmoo); putch("tsmoo","f",&tsmoo);
  smute = 2.0;  getch("smute","f",&smute); putch("smute","f",&smute);
  if (smute<=1.0) seperr("smute > 1 is required\n");

  /* compute constants */
  smooth = tsmoo/dt+1;
  smooth = (smooth/2)*2+1;
  i0out = (t0out-t0)/dt;
  if (i0out<0) i0out = 0;
  idout = dtout/dt+0.5;
  if (idout<1) idout = 1;
  if (i0out+(ntout-1)*idout>=nt)
    seperr("output time-axis cannot exceed input time-axis\n");
  t0out = t0+i0out*dt;
  dtout = idout*dt;
  tsmoo = (smooth-1)*dt;
  smooth= tsmoo/dt;

  sprintf(verbose,"no"); getch("verbose","s",verbose);
  vv = (*verbose == 'y');
  /* print parameters */
  if(vv) {
    fprintf (stderr,"\tnt=%d dt=%g t0=%g nf=%d df=%g f0=%g\n",
      nt,dt,t0,nf,df,f0);
    fprintf (stderr,"\tntout=%d dtout=%g t0out=%g\n",
      ntout,dtout,t0out);
    fprintf(stderr,"\tng=%d dg=%d g0=%d tsmoo=%g smute=%g\n", ng,dg,g0,tsmoo,smute);
    fprintf(stderr,"\tvelopt=%s\n",velopt);
    if(velopt[0]=='v')
      fprintf (stderr,"\tnv=%d dv=%g v0=%g last v=%g\n",
        nv,dv,v0,v0+(nv-1)*dv);
    else if(velopt[0]=='s')
      fprintf (stderr,"\tnw=%d dw=%g w0=%g last w=%g\n",
        nw,dw,w0,w0+(nw-1)*dw);
    else seperr("unknown velopt\n");
    }

  /* Part 1 of revissions allocate arrays and
     set some arrays that are data independant*/

  input=(float *) malloc( sizeof(float) * nt * nf);  
  output=(float *) malloc( sizeof(float) * ntout * nv);  
  num =(float *) malloc (sizeof(float) * nt);
  denom =(float *) malloc (sizeof(float) * nt);
  offslow =(float *) malloc (sizeof(float) * nf);
  count =(int *) malloc (sizeof(int) * nt);
  mutetime =(int *) malloc (sizeof(int) * nt);


 icdp=ng/dg;   
  if((ng/dg)*dg != ng) icdp=icdp+1;
putch("n3",    "d",&icdp);
  hclose();

  icdp = g0+ng;
  for (ig=g0; ig<icdp; ig += dg)
    {
    if(vv) fprintf(stderr,"ig=%d\n",ig);
    if( sseek("in",nt*nf*ig*esize,0) == -1)
      seperr("sseek error ig=%d\n",ig);
    if( sreed("in",input,nt*nf*esize) != nt*nf*esize)
      seperr("read error ig=%d\n",ig);

  /* THIS IS THE START OF THE REVISIONS -Bob*/
  


  /* Loop over velocities */
  for(iv=0; iv < nv; iv++){
    for(i1=0; i1 < nt; i1++) { num[i1]=0.; denom[i1]=0.; count[i1]=0;}
    
  /* Loop over offsets setting */
  if(velopt[0]=='v')
    for(i1=0; i1 < nf; i1++)
       offslow[i1]=(f0 + df * i1)/(v0+ dv * iv);
  else
    for(i1=0; i1 < nf; i1++)
       offslow[i1]=(f0 + df * i1)*(w0+ dw * iv);

  for(i1=0; i1 < nf; i1++){
     tempr=sqrt(offslow[i1]*offslow[i1]/(smute*smute-1.));
    mutetime[i1]=(tempr-t0)/dt +1.5;
  }

    /*Loop over offsets */
    for(i2=0; i2 < nf; i2++){
      for(i1=0; i1< nt; i1++){
        tau=t0+i1 * dt;
        time= sqrt((double)(offslow[i2]*offslow[i2]+tau*tau));
        tuse=(time-t0)/dt;
        fract=((time-t0)/dt)-tuse;
        if (tuse >= 0 && tuse < nt && time >0.){
          if (tuse < nt-1)
            tempr=(input[tuse+ nt* i2]*(1.-fract)+ 
            fract*input[tuse+1+i2*nt]);
          else if (tuse==nt-1) tempr=input[tuse+nt*i2];
          if (tempr!=0.){
            num[i1]=num[i1] + tempr;
            denom[i1]=denom[i1] + tempr*tempr;
            count[i1]=count[i1] + 1;
          }

        }

      }
    }
    for(i1=0; i1 < ntout; i1++){
      numsum=0; densum=0;
      time=t0out + dtout * i1;
      begi=(time - tsmoo - t0) /dt;
      endi=(time + tsmoo - t0) /dt;
      if (begi<0) begi=0;
      if (endi>=nt) endi=nt-1;
      for(i0=begi;i0<endi; i0++){
        numsum=numsum+num[i0]*num[i0];
        densum=densum+denom[i0]*count[i0]*1.0;
      }
      if(densum==0.)  output[i1+iv*ntout]=0.;
      else output[i1+ntout*iv]=numsum/densum;
    }
  }
  if(esize*ntout*nv!= srite("out",output,esize*ntout*nv))
    seperr("trouble writing out dataset \n");
}
  sep_end_prog();
  return EXIT_SUCCESS;
}
