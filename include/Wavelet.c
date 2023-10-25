/*$

=head1 NAME

Wavelet - wavelet generation

=head1 SYNOPSIS

Wavelet parameters [ < in.H ] > out.H

=head1 DESCRIPTION

Wavelet generation program usually used for modeling programs

=head1 INPUT PARAMETERS

=over 4

=item  nt,n1  - integer 

dimension of time axis
parameter n1wavelet = n1out also output to header

=item  nx,n2  -integer 

[1] number of input traces to use (see wavelet=data)

=item  np,n3  -integer 

[1]  number of input planes to use (see wavelet=data)

=item  dt     -real    

[.004] sample rate
(parameter d1wavelet = d1out also output to header)

=item  domain -char*    

[time]  domain option: time,ctime,frequency,spectrum
time      = nt real values representing the wavelet
ctime     = nt complex values representing the wavelet
frequency = nt/2+1 complex values representing frequency domain wavelet
spectrum  = nt+2 real values, first half are normalized
            spectrum, second half are normalized phase
            angle which is the phase angle divided by
            pi. Usually used for quick check plots.
(parameter domainwavelet = domain also output to header)

=item  wavelet  -char*  

[spike]  type of wavelet
spike   = spike in time domain equals white spectrum
bandpass= butterworth bandpass filter, sharpness of
           frequency cutoffs is controlled by the order
ricker0 = gaussian curve
ricker1 = first derivative of gaussian curve
ricker2 = second derivative of gaussian curve
data    = amplitude spectrum is average of
          nx*np input traces from the in file
zero    = null wavelet (ie: all zeroes)

=item phase -char*    

[none]      phase options: none,minimum,degrees
none    = no modification of phase
minimum = minimum phase
min2zero = minimum to zero phase conversion
degrees = numerical value representing degrees of
          constant phase shift from specified wavelet

=item  fund -real      

[40]     fundamental frequency of ricker wavelet
         (actually relates to gaussian half-width)
=item  flow -real      

[10]   low  cutoff frequency of butterworth filter

=item fhigh -real      

[60]   high cutoff frequency of butterworth filter

=item  order-integer   

[6]    order of butterworth filter
       (1 is average sharpness, > 1 is sharper, < 1 is smoother)
       (sets both high and low frequency cutoffs)

=item   orderlow-integer 

[6] low  cut order of butterworth filter (overrides order)

=item  orderhigh-integer 

[6] high cut order of butterworth filter (overrides order)

=item  tdelay - real    

additional time delay

=item  bell -real       

[nt/4*dt]    bell window half width, < dt/2 for no bell window

=item  boxcar -integer  

[pnt*dt]    boxcar window width

=back

=head1  CATEGORY

B<seis/model>

=cut
 >*/

/*
  KEYWORDS      modeling  wavelet  source ricker minimum phase

 SEE ALSO

 WHERE
       ./cube/seis/Wavelet.rst

%

 Author - Peter Mora	3/7/84
  1-22-88     Clement     putch esize
  revised - Ray Abma 25 March 1992
            Expanded some buffers to match the old version.
  revised - Biondo Biondi 2 May 1992
            Increased maximum size of trace and made ALL buffer declaration
		dependent on the parameter MAXN
  revised - Ray Abma 2 Feb. 1993
            Made the buffers dynamically allocate, cleaned up.
  revised - Dave Nichols 21 Apr. 1994
            Added min to zero phase conversion option
  revised - Stew Levin 5 May 1996
            Worked around LINUX g77 bug by moving long character
            strings from subroutine calls into separate, explicitly
            declared character variables.
  revised - James Rickett September 1999
            Converted to C

 ----------
  Keyword:  modeling  wavelet  source ricker minimum phase
 ----------
*/

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

#include <seplib.h>
#include <sepcube.h>

int wavlet(complex *s,int nt,float dt,int nx,int np,char *domain,
	    char *wave,char *phase,float fund,float flow,float fhigh,
	    int orderl,int orderh,float tdelay,float bell,float boxcar,
	    float degrees,float *rs,complex *cs);

int pad2(int);

int main (argc, argv, envp) 
int argc;
char  **argv, **envp;
{

  complex *cs, *s;
  float *rs;

  int ier,nt,n0,nx,np,esize,n1out,i;
  float dt,ot,d1out, fund, flow,fhigh;
  char ascii[20], domain[20], wave[20], phase[20];
  int order,orderl,orderh;
  float tdelay,bell,boxcar,degrees;
  
  initpar (argc, argv); 
sep_add_doc_line("NAME");
sep_add_doc_line("    Wavelet - wavelet generation");
sep_add_doc_line("");
sep_add_doc_line("SYNOPSIS");
sep_add_doc_line("    Wavelet parameters [ < in.H ] > out.H");
sep_add_doc_line("");
sep_add_doc_line("DESCRIPTION");
sep_add_doc_line("    Wavelet generation program usually used for modeling programs");
sep_add_doc_line("");
sep_add_doc_line("INPUT PARAMETERS");
sep_add_doc_line("    nt,n1 - integer");
sep_add_doc_line("        dimension of time axis parameter n1wavelet = n1out also output to");
sep_add_doc_line("        header");
sep_add_doc_line("");
sep_add_doc_line("    nx,n2 -integer");
sep_add_doc_line("        [1] number of input traces to use (see wavelet=data)");
sep_add_doc_line("");
sep_add_doc_line("    np,n3 -integer");
sep_add_doc_line("        [1] number of input planes to use (see wavelet=data)");
sep_add_doc_line("");
sep_add_doc_line("    dt -real");
sep_add_doc_line("        [.004] sample rate (parameter d1wavelet = d1out also output to");
sep_add_doc_line("        header)");
sep_add_doc_line("");
sep_add_doc_line("    domain -char*");
sep_add_doc_line("        [time] domain option: time,ctime,frequency,spectrum time = nt real");
sep_add_doc_line("        values representing the wavelet ctime = nt complex values");
sep_add_doc_line("        representing the wavelet frequency = nt/2+1 complex values");
sep_add_doc_line("        representing frequency domain wavelet spectrum = nt+2 real values,");
sep_add_doc_line("        first half are normalized spectrum, second half are normalized phase");
sep_add_doc_line("        angle which is the phase angle divided by pi. Usually used for quick");
sep_add_doc_line("        check plots. (parameter domainwavelet = domain also output to");
sep_add_doc_line("        header)");
sep_add_doc_line("");
sep_add_doc_line("    wavelet -char*");
sep_add_doc_line("        [spike] type of wavelet spike = spike in time domain equals white");
sep_add_doc_line("        spectrum bandpass= butterworth bandpass filter, sharpness of");
sep_add_doc_line("        frequency cutoffs is controlled by the order ricker0 = gaussian");
sep_add_doc_line("        curve ricker1 = first derivative of gaussian curve ricker2 = second");
sep_add_doc_line("        derivative of gaussian curve data = amplitude spectrum is average of");
sep_add_doc_line("        nx*np input traces from the in file zero = null wavelet (ie: all");
sep_add_doc_line("        zeroes)");
sep_add_doc_line("");
sep_add_doc_line("    phase -char*");
sep_add_doc_line("        [none] phase options: none,minimum,degrees none = no modification of");
sep_add_doc_line("        phase minimum = minimum phase min2zero = minimum to zero phase");
sep_add_doc_line("        conversion degrees = numerical value representing degrees of");
sep_add_doc_line("        constant phase shift from specified wavelet");
sep_add_doc_line("");
sep_add_doc_line("    fund -real");
sep_add_doc_line("        [40] fundamental frequency of ricker wavelet (actually relates to");
sep_add_doc_line("        gaussian half-width) =item flow -real");
sep_add_doc_line("");
sep_add_doc_line("        [10] low cutoff frequency of butterworth filter");
sep_add_doc_line("");
sep_add_doc_line("    fhigh -real");
sep_add_doc_line("        [60] high cutoff frequency of butterworth filter");
sep_add_doc_line("");
sep_add_doc_line("    order-integer");
sep_add_doc_line("        [6] order of butterworth filter (1 is average sharpness, > 1 is");
sep_add_doc_line("        sharper, < 1 is smoother) (sets both high and low frequency cutoffs)");
sep_add_doc_line("");
sep_add_doc_line("    orderlow-integer");
sep_add_doc_line("        [6] low cut order of butterworth filter (overrides order)");
sep_add_doc_line("");
sep_add_doc_line("    orderhigh-integer");
sep_add_doc_line("        [6] high cut order of butterworth filter (overrides order)");
sep_add_doc_line("");
sep_add_doc_line("    tdelay - real");
sep_add_doc_line("        additional time delay");
sep_add_doc_line("");
sep_add_doc_line("    bell -real");
sep_add_doc_line("        [nt/4*dt] bell window half width, < dt/2 for no bell window");
sep_add_doc_line("");
sep_add_doc_line("    boxcar -integer");
sep_add_doc_line("        [pnt*dt] boxcar window width");
sep_add_doc_line("");
sep_add_doc_line("CATEGORY");
sep_add_doc_line("    seis/model");
sep_add_doc_line("");

  doc(SOURCE);

  sep_begin_prog();
  if(fetch("n1","i",&nt)==0) (void) seperr("n1 is required");

  n0=nt; nt=pad2(nt);  /* pad to nearest power of 2 */

  if (fetch("n2","i",&nx)==0) nx=1;
  if (fetch("n3","i",&np)==0) np=1;

  i=1;
  ier=putch("n2","d",&i);
  ier=putch("n3","d",&i);

  if (fetch("d1","r",&dt)==0) dt=0.004;
  if ((dt<1.e-10) || (dt>1.e10)) (void) seperr("bad d1");
  if (fetch("o1","r",&ot)==0) ot=0.0;
  ier=putch("o1","r",&ot);

  if (fetch("ascii",  "s",ascii ) == 0) strcpy(ascii,"n");
  if (fetch("domain", "s",domain) == 0) strcpy(domain,"time");
  if (fetch("wavelet","s",wave  ) == 0) strcpy(wave,"spike");
  if (fetch("phase",  "s",phase ) == 0) strcpy(phase,"none");
  if ( ( strcmp(phase,"minimum")!=0 && strcmp(phase,"min2zero")!=0 && 
	 strcmp(phase,"none")!=0                                      ) &&
       ( fetch("phase","f",&degrees)==0 )) degrees=0.0;
    

  if (fetch("fund", "r",&fund )==0) fund=40.;
  if (fetch("flow", "r",&flow )==0) flow=10.;
  if (fetch("fhigh","r",&fhigh)==0) fhigh=60.;

  orderl=6 ; orderh=6;   order=6;
  if(fetch("order","d",&order)!=0) {
    orderl=order ; orderh=order;
  }
  ier=fetch("orderlow","d",&orderl);
  ier=fetch("orderhigh","d",&orderh);

  tdelay=0.;            ier=fetch("tdelay","r",&tdelay);
  bell=nt/4*dt;         ier=fetch("bell","r",&bell);
  boxcar=nt*dt;         ier=fetch("boxcar","r",&boxcar);

  (void) putlin("Wavelet generation");
   
  if (strcmp(domain,"ctime")==0 || strcmp(domain,"frequency")==0) esize=8;
  else  esize=4;
  ier=putch("esize","i",&esize);
   
  ier=putch("ascii","s",&ascii);
  n1out=n0;
  ier=putch("given n1","i",&n0);
  if(n0!=nt) {
    (void) putlin("given n1 not a power of 2, new n1 computed\n");
    n1out=nt ; ier=putch("computed n1","i",&n1out);
  }
  ier=putch("d1","r",&dt);
  if(strcmp(domain,"spectrum")==0||strcmp(domain,"frequency")==0) {
    putlin("new output n1 computed for spec/freq domain");
    n1out=nt/2 ; ier=putch("computed n1","i",&n1out);
  }
  if(strcmp(domain,"ctime")==0) {
    putlin("new output n1 for complex time series (n1 reals)");
    n1out=nt ; ier=putch("computed n1","i",&n1out);
  }
  d1out=dt;
  if(strcmp(domain,"spectrum")==0) {
    putlin("new d1 computed in frequency in cycles per sec");
    d1out=1/(nt*dt) ; ier=putch("computed d1","r",&d1out);
  }
  else if(strcmp(domain,"frequency")==0) {
    putlin("new d1 computed in radians per sec");
    d1out=2*pi/(nt*dt) ; ier=putch("computed d1","r",&d1out);
  }
  if(strcmp(wave,"data")==0) {
    ier=putch("given n2 ","i",&nx);
    ier=putch("given n3 ","i",&np);
  }
  
  /*  what this one all about  */
  if(strcmp(domain, "time")==0) { ier=putch("n1","i",&n0); }
  if(strcmp(domain,"ctime")==0) { ier=putch("n1","i",&n0); }
  ier=putch("domain","s",domain);
  ier=putch("n1wavelet","i",&n1out);
  ier=putch("d1wavelet","r",&d1out);
  ier=putch("domainwavelet","s",domain);
  ier=putch("wavelet","s",wave);
  ier=putch("phase","s",phase);
  ier=putch("degrees","r",&degrees);
  ier=putch("fund","r",&fund);
  ier=putch("flow","r",&flow);
  ier=putch("fhigh","r",&fhigh);
  ier=putch("order","d",&order);
  ier=putch("orderlow","d",&orderl);
  ier=putch("orderhigh","d",&orderh);
  ier=putch("tdelay","r",&tdelay);
  ier=putch("bell","r",&bell);
  ier=putch("boxcar","r",&boxcar);
  
  /* allocate some space */
  s= (complex*) malloc(nt*sizeof(complex));
  cs=(complex*) malloc(nt*sizeof(complex));
  rs=(float*)   malloc(nt*sizeof(float  ));
  hclose();
  
  nt=n0;

  ier= wavlet(s,nt,dt,nx,np,domain,wave,phase,fund, 
	      flow,fhigh,orderl,orderh,tdelay,bell,boxcar,degrees,rs,cs);
  
  /* write out wavelet */
  if      (strcmp(domain,"time" )==0)     ier=srite("out",rs,nt*4);
  else if (strcmp(domain,"ctime")==0)     ier=srite("out",s,nt*8);
  else if (strcmp(domain,"frequency")==0) ier=srite("out",s,n1out*8);
  else                                    ier=srite("out",rs,n1out*4);
  
  sep_end_prog();
  return 0;
}


int pad2(int n)
{
  int i;
  i=1;
  while (i<n) { i=2*i; }
  return i;
}





/*
  wavlet - Wavelet generation program usually used for modeling programs

  infd		input unit number (required only for wavelet=data)
  s		complex wavelet
  nt		dimension (input dimension, nearest >= power of 2 returned)
  dt=.004		sample rate
  nx		number of traces in data file (see wavelet=data)
  np		number of planes in data file (see wavelet=data)
  domain=time	domain option: time,ctime,frequency,spectrum
  time	  = nt real values representing the wavelet
  ctime	  = nt complex values representing the wavelet
  frequency = nt/2+1 complex values representing the frequency domain wavelet
  spectrum  = nt+2 real values, first half are normalized
              spectrum, second half are normalized phase angle
              which is the phase angle divided by pi
  wavelet=spike	
  wavelet options: spike,bandpass,ricker0,ricker1,ricker2,data
			spike	= spike in time domain equals white spectrum
			bandpass= butterworth bandpass filter, sharpness of
				  frequency cutoffs is controlled by the order
			ricker0 = gaussian curve
			ricker1 = first derivative of gaussian curve
			ricker2 = second derivative of gaussian curve
			data	= amplitude spectrum is average of
				  nx*np input traces from the in file
	phase=none	phase options: none,minimum,degrees
			none	= no modification of phase
			minimum = minimum phase
			degrees = numerical value representing degrees of
				  constant phase shift from specified wavelet
	fund=40		fundamental frequency of ricker wavelet
			(actually relates to gaussian half-width)
	flow=10		low  cutoff frequency of butterworth filter
	fhigh=60	high cutoff frequency of butterworth filter
	orderl=10	order of butterworth filter at low  frequency end
	orderh=20	order of butterworth filter at high frequency end
	tdelay=0	additional time delay
	bell=nt/4*dt	bell window half width,   < dt/2 for no bell window
	boxcar=nt*dt	boxcar window width
     */	
  /*
       Author - Peter Mora
  */

#include<string.h> 
#include<math.h>
#include<sepmath.h>
#include<sepfft.h>
#include<seplib.h>
int wavlet(complex *s,int nt,float dt,int nx,int np,char *domain,
	    char *wave,char *phase,float fund,float flow,float fhigh,
	    int orderl,int orderh,float tdelay,float bell,float boxcar,
	    float degrees,float *rs,complex *cs)

{
  int stat,it,ip,ix,iw,n0;
  double dw,w;
  char test[20],ider,nder;

  float phi,hwidth,alpha,box;
  int jt;
  complex phaze;

  extern int pad2(int);
  extern void rfft(complex*,int,int);

  double t,t1,t2;
  double smax,save;

  double scale;

  int l;

  extern void normalize(complex *s,int nt);
  extern void bandpass(complex *s,float flow,float fhigh,
	      int orderl,int orderh,int nt,float dt);
  extern int kol1(complex *,int,int);
  
  strcpy(test,wave);test[6]='\0';
  n0=nt ; nt=pad2(nt);

  /* data */
  if(strcmp(wave,"data")==0) {
    for (it=0; it<nt; it++) { s[it].re=0.; s[it].im=0.; }
    for (ip=0; ip<np; ip++) { 
      for (ix=0; ix<nt; ix++) {
	stat=sreed("in",rs,n0*4);                           /* read in data */
	for (it=0; it<n0; it++) { cs[it].re=rs[it]; cs[it].im=0.; } /* rtoc */
	for (it=n0;it<nt; it++) { cs[it].re=0.;     cs[it].im=0.; }  /* pad */
	(void) rfft(cs,nt,1);                                        /* fft */
	for (it=0;it<nt/2+1;it++) { 
	  s[it].re += sccabs(cs[it]);                            /* sum cabs */
	}}}
    for (it=0;it<nt/2+1;it++) { s[it].re /= (nx*np); }         /* normalize */
    (void) rfft(s,nt,-1);                                           /* ifft */
  }

  /* spike */
  if(strcmp(wave,"spike")==0) { 
    for (it=1; it<nt; it++) {
      s[it]=scmplx(0.,0.);
    } 
    s[0]=scmplx(1.,0.);
  }

  /* bandpass */
  else if(strcmp(wave,"bandpass")==0) { 
    for (it=0; it< nt/2+1; it++) { s[it]=scmplx(1.,0); }     /* flat spectrum */
    (void) bandpass(s,flow,fhigh,orderl,orderh,nt,dt);          /* bandpass */
    (void) rfft(s,nt,-1);                             /* back to time domain */
    (void) normalize(s,nt);
  }

  /* ricker */ 
  else if((strcmp(test,"ricker")==0) || (strcmp(test,"bicker")==0)) {
    w=2*sqrt(2.)*fund ;
    t=0;
    for (it=0; it<nt; it++) {
      t1=t*w ; t2=t1*t1;
      s[it].im=0.;
      if(t2<20.)  s[it].re=exp(-t2);      /* spectrum = gaussian */
      else        s[it].re=0. ;
      if(sccabs(s[it]) < 1.e-10) s[it]=scmplx(0.,0.);
      if(it==nt/2) {t=-t ;}
      t=t+dt;
    }
    
    nder=wave[6]-48;     /* number of times to differentiate */
    if(nder>0) {
      (void) rfft(s,nt,1);
      for (iw=0; iw<nt/2+1; iw++) {
	if(sccabs(s[iw])<1.e-5) { s[iw]=scmplx(0.,0.); } 
      }
      dw=2*pi/(nt*(double) dt);
      for (ider=0; ider<nder; ider++) {
	w=0. ; 
	for (iw=0; iw<nt/2; iw++) {
	  s[iw]=scmult(s[iw],scmplx(0.,(float) (-w))); /* multiply by (-iw) */
	  w += dw;
	} 
	s[nt/2]=scmplx(0.,0.); 
      }
      
      (void) rfft(s,nt,-1);
      save=0. ; smax=(double) s[0].re;
      for (it=0; it<nt/2; it++) {
	smax=MAX(smax,fabs((double) s[it].re)) ; 
	save += (double) s[it].re ;
      }
      save=save/nt ; 
      for (it=0; it<nt; it++) { 
	s[it]=scmplx((s[it].re)/smax,s[it].im/smax); 
      } 
    }
  }

  /* zero */
  else if(strcmp(wave,"zero")==0) { 
    for (it=0; it<nt; it++) {  s[it]=scmplx(0.,0.);  } 
  }

  /*  bandpassed ricker */
  if(strcmp(test,"bicker")==0) {
    (void) rfft(s,nt,1);                           /* into frequency domain */
    (void) bandpass(s,flow,fhigh,orderl,orderh,nt,dt);          /* bandpass */
    (void) rfft(s,nt,-1);                            /* back to time domain */
    (void) normalize(s,nt); 
  }

  /*  additional phase options */
  (void) rfft(s,nt,1);                           /* into frequency domain */
  
  if(strcmp(phase,"minimum")==0 || strcmp(phase,"min2zero")==0) { 
    l=MAX(((int) (bell/dt+0.5)),0);
    (void) kol1(s,nt,l);
  } 

  /* degrees option */
  if(fabs(degrees) > 0.001) {
    phi=degrees*pi/180.;
    phaze=sciexp(-phi); 
    for (it=0;it<nt;it++) s[it]=scmult(s[it],phaze);
  }

  /*  min to zero phase conversion, reverse phase and remove amplitude term */
   if( strcmp(phase,"min2zero")==0 )
     for (it=0;it<nt;it++) 
       s[it]=scsmult(sconjg(s[it]),1./(sccabs(s[it])+1.0e-5));
   /* do it=1,nt { s(it) = sconjg(s(it))/(cabs(s(it))+1e-5) } */

   /*  window options */
   if ( bell>dt/2  || (boxcar>dt/2 && boxcar<(nt*dt-dt/2))) {
     (void) rfft(s,nt,-1);
     if (bell>dt/2 && strcmp(phase,"minimum")!=0) {
       hwidth=sqrt(-log(.5)/pi) ; alpha=pi*(hwidth/bell)*(hwidth/bell);
       t=0;
       for (it=0;it<nt;it++) {
	    s[it]=scsmult(s[it],exp(-alpha*t*t)); 
	    if(it==nt/2) t=-t ; 
	    t+=dt;
       }
     }
     if (boxcar>dt/2 && boxcar<nt*dt-dt/2) {
       if (strcmp(phase,"minimum")==0) { box=boxcar ; jt=1; }
       else { box=boxcar  ; jt=-1; }
       t=0;
       for (it=0;it<nt;it++) {
	 if(abs(t)>box) s[it]=scmplx(0.,0.) ; 
	 if(it==nt/2) t *= jt ; 
	 t += dt ;
       }
     }
     (void) rfft(s,nt,1);
   }

   /*  additional time delay */
   if(tdelay!=0.) {
     dw=2*pi/(nt*(double) dt); w=0.;
     for (it=0; it<nt; it++) { 
       s[it]=scmult( s[it], scexp(scmplx(0., (float) (w*tdelay)))); 
       if(it==nt/2) w= (-w); 
       w=w+dw;
     }
   }

   /* change to output domain and arrange data into output domain */

   if(strcmp(domain,"time")==0 || strcmp(domain,"ctime")==0) 
     (void) rfft(s,nt,-1);        /* inverse fft */

   if(strcmp(domain,"time")==0)  /* take real part */
     for (it=0; it<nt; it++)
       rs[it]=s[it].re;

   else if(strcmp(domain,"spectrum")==0) {
     scale=1.0e-20;
     for (it=0; it<nt/2+1; it++) 
       scale=MAX(sccabs(s[it]),scale);
     
     scale=1./scale;
     for (it=0; it<nt/2+1; it++) { 
       rs[it]=sccabs(s[it])*scale;
       if(sccabs(s[it])<1.0e-20) rs[it+nt/2+1]=0.;
       else            rs[it+nt/2+1]=atan2(-s[it].im,(s[it].re))/pi ;
     }
   }
  
   return 0;
}

/*
  
  Kolmogoroff spectral factorization to generate a minimum phase wavelet
	
  f fourier transformed time series
  n length of f
  l halfwidth in samples of required wavelet. basically multiplies time
    wavelet by gaussian of this halfwidth. effectively this smooths the
    spectrum. l=0 if infinity halfwidth required
    
*/

int kol1(complex *f,int n,int l)
{
  int i;
  double hwidth,alpha,ampmax,ampmin;

  extern void rfft(complex*,int,int);

#define ACC 0.0001

  /* get (sqrt) autocorrelation spectrum */
  for (i=1; i<n/2+1; i++){
    f[i]=scmplx(sccabs(f[i]),0.);
    f[n-i].re=f[i].re;
    f[n-i].im=f[i].im;
  }

   for(i=0; i < n; i++) f[i].re=f[i].re*sqrt((double)n)/2.;

  /* smooth autocorrelation spectrum if required */
  if(l!=0) {
    (void) rfft(f,n,-1);
    hwidth=sqrt(-log(.5)/pi); 
    alpha=pi*(hwidth/((float) l))*(hwidth/((float) l));
    for (i=1; i<n/2+1; i++) {
      f[i]=scmplx(f[i].re* exp(-alpha*(i)*(i)),0.); 
      f[n-i].re=f[i].re;
      f[n-i].im=f[i].im;
    }
    (void) rfft(f,n,1);
  }

   /* take the log */
   ampmax=0. ; 
   for (i=0; i<n/2+1; i++){
     ampmax=MAX(f[i].re,ampmax);
   }

   ampmin=ampmax*ACC; 
   for (i=1; i<n/2+1; i++){
      f[i]=scmplx((float)log((double)(f[i].re+ampmin)),0.);
      f[i]=scmplx(f[i].re,0.);
   }

      f[0]=scmplx((float)log((double)(f[0].re+ampmin)),0.);
   /* Hilbert transform the log */
   for(i=1; i < n/2+1; i++) f[n-i].re=f[i].re;
   for(i=0; i < n; i++) f[i].re=f[i].re*sqrt(1./(double)n)*2.;
   (void) rfft(f,n,-1);

   for (i=1; i<n/2; i++) {
     f[i]=scadd(f[i],f[i]);   /* (multiplying by 2 sqr's spectrum) */
     f[n-i]=scmplx(0.,0.);
   }

   (void) rfft(f,n,1);
   for(i=1; i < n/2+1; i++) f[n-i].re=f[i].re;
   for(i=0; i < n; i++) f[i].re=f[i].re*sqrt((double)n)/2.;
   for(i=0; i < n; i++) f[i].im=-f[i].im*sqrt((double)n)/2.;

   /* Take the exp */
   for (i=0; i<n; i++){
     f[i]=sconjg(scsub(scexp(f[i]),scmplx(ampmin,0.)));
   }
   for(i=0; i < n; i++) f[i].re=f[i].re/sqrt((double)n);
   for(i=0; i < n; i++) f[i].im=f[i].im/sqrt((double)n);

   return 0;
}



void rfft(complex *x,int n, int iop)
{
  cefft(x, n, iop, sqrt(1./n));
  return;
}

/*
 *  Butterworth bandpass filter
 */
void bandpass(complex *s,float flow,float fhigh,
	      int orderl,int orderh,int nt,float dt)
{
  int it;
  float df,fzer,fny,fnyq,f,ordl,ordh,val;
float v,amp,cosf;
  df=1/(nt*dt); fzer=df*.5; fny=.5/dt ; fnyq=fny+fzer ; f=0.;
  fprintf(stderr,"I SEE %f %d %f \n",dt,nt,df);
  /*    ordl=flow*orderl; ordh=orderh*(fny-fhigh); */
 /*
 ordl=orderl; ordh=orderh;
  for (it=0; it< nt/2+1; it++) {
    if(fhigh<fnyq) {                                            
      val=MIN(30., log( MAX( 1.0e-10, (fny-f)/(fny-fhigh) ))*ordh  );
      s[it]=scsmult(s[it],(1-1/(1+exp(val))));
    }
    if(flow>fzer)  {                                              
      val=MIN(30.,log(MAX(1.0e-10,f/flow))*ordl);
      s[it]=scsmult(s[it],(1-1/(1+exp(val))));
    }
    f += df;
  }
*/
  for (it=0; it< nt/2+1; it++) {
        amp= MAX(1e-3,sqrtf(s[it].re*s[it].re+s[it].im*s[it].im));
amp=1.;
   // if(f<1.) {
   //   //s[it].re=0.; s[it].im=0.;
  // }
    if(f<2.){
      s[it].re=s[it].re*cos( (2.-f)/1.*atan(1.)*2.)/amp;
      s[it].im=s[it].im*cos (( 2.-f)/1.*atan(1.)*2.)/amp;
     fprintf(stderr,"CHECK 0 %d %f %f \n",it,(2-f)/2.*atan(1.)*2.,amp);
    }
    else if(f<90.){
      s[it].re=s[it].re/amp;
      s[it].im=s[it].im/amp;
    }
    else if(f<100.){
       s[it].re=s[it].re*cos( (f-90.)/10.*atan(1.)*2.)/amp;
      s[it].im=s[it].im*cos (( f-90.)/10.*atan(1.)*2.)/amp;
      fprintf(stderr,"CHECK THIS %d %f \n",it,(f-90.)/10.*atan(1.)*2.);
    }
    else{
     s[it].re=0.; s[it].im=0.; 
    
    }
    f+=df;
   fprintf(stderr,"CHECK %d %f %f %f %f\n",it,f,s[it].re,s[it].im,amp);
  }
  return;
}

void normalize(complex *s,int nt)
{
  double scale;
  int it;     
  scale=1.0e-30 ; for (it=0;it<nt;it++) { scale=MAX(fabs(s[it].re),scale); } 
  scale=1/scale ; for (it=0;it<nt;it++) { s[it]=scsmult(s[it],scale); } 
}
