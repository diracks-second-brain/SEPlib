/*$

=head1 NAME

Tpow - scales on the time axis

=head1 SYNOPSIS

Tpow par= < in.H > out.H

=head1 DESCRIPTION

   Select a power of t for seismogram scaling
   out(t)  =  data(t)* t**tpow

=head1 INPUT PARAMETERS

=over 4

=item  n1,n2,n3,n4,n5,n6,n7 - int 

       [nt],[nx],[1] usual seismic data t inner

=item  d1,d2     -float 

       [dt],[dx] samplinginterval

=item  o1,o2     -float 

       [ot],[ox] [first sample time]

=item  tpow      -float  

       power of time

=item  panel     -char* 

       [1]: (default) 1st panel is used for estimating tpow.
       When panel=all, all panels are used.

=item  tmin      -float 

			[o1]:   time of the first sample to be used in estimating tpow. 

=item  tmax      -float 

       [4.]: end of region used on time axis

=item  ofdep     -char* 

       [unknown]      unknown: automatic picking of first
         arrivals will be applied to compute the muting parameters
       no:  data are offset-independent 
       yes: data are offset-dependent 

=item  v0-  float      

       (km/s) apparent |dx/dt|. muting 
        before first arrival time will 
        be applied in estimating the 
        medians. when no v0 is given
        and ofdep is not 'no', automatic
        picking will be applied 

=item  nxmin - int    

       [1]: the first trace used for Tpow 

=item  nxmax=nx       

       the last trace used. For 3-d data,
       trace nxmin to trace nxmax are 
       used on each plane if panel=all.

=item  eps  - float   

       [0.05]: (default) iteration of estimating dtpow stops
       when abs(dtpow) < eps.

=item  idt  - integer 

       [2]: subsample rate (integer) of time for tpow
       estimating

=item  idx  - integer 

       [1]: subsample rate (integer) of trace for tpow estimating

=item  iwind - integer 

       [9]: window length used for calculating L-1 energy in autopicking

=item  perc  - float   

       [10.]: when L-1 norm of present window exceeds
       that of the previous one by 
       'perc', first arrival is picked 

=back

=head1 COMMENTS

=head2 METHOD

  m02 * (.25*tmax)**(dtpow) = m24 * (.75*tmax)**(dtpow)
  tpow = tpow + dtpow
  where 
    m02 --- median over (0, tmax/2)    of | data(t) | * t**tpow
    m24 --- median over (tmax/2, tmax) of | data(t) | * t**tpow

=head1 CATEGORY

B<seis/filter>

=head1 COMPILE LEVEL

DISTR

=cut

>*/
/*
  
 KEYWORDS gain tpow : Gpow

 WHERE
  ./cube/plot/Tpow.c

*/
/*
EDIT HISTORY
  8-25  jon  generalized 4 sec to tmax.
  8-25  jon  given tpow now getch'ed instead of fetch'ed.
  9-24    li      modified to accept input from pipe
  11-11   li      add options tmin, ofdep, v0, nxmin and nxmax 
      to allow program to be used to correctly estimate tpow 
      even when input data is offset-dependent.
  11-29   li  automatic first-arrival picking subroutine added 
  1-8-92  jon  eliminate nt,nx anachronisms.
 */
/*
 * Keyword: gain tpow : Gpow
 */

#define XXSZ 1048576  
char panel[20]="first";
char ofdep[20]="unknown";



void esttpow(float *xx,int *antt,int *anxx,int *an3,float *adttpow,float *atmin,float *atpow,int *aiofdp,float *av0,float *at0x0,float *aeps,float *axmin,float *adxtpw);
float cent (float p,float *x,int n);
void fstpick(float *x,int *ant,int *aitpeak,int *alwind,float *aperc);

#define MY_SEP_DOC \
 sep_add_doc_line("NAME");\
 sep_add_doc_line("    Tpow - scales on the time axis");\
 sep_add_doc_line("");\
 sep_add_doc_line("SYNOPSIS");\
 sep_add_doc_line("    Tpow par= < in.H > out.H");\
 sep_add_doc_line("");\
 sep_add_doc_line("DESCRIPTION");\
 sep_add_doc_line("       Select a power of t for seismogram scaling");\
 sep_add_doc_line("       out(t)  =  data(t)* t**tpow");\
 sep_add_doc_line("");\
 sep_add_doc_line("INPUT PARAMETERS");\
 sep_add_doc_line("    n1,n2,n3,n4,n5,n6,n7 - int");\
 sep_add_doc_line("               [nt],[nx],[1] usual seismic data t inner");\
 sep_add_doc_line("");\
 sep_add_doc_line("    d1,d2 -float");\
 sep_add_doc_line("               [dt],[dx] samplinginterval");\
 sep_add_doc_line("");\
 sep_add_doc_line("    o1,o2 -float");\
 sep_add_doc_line("               [ot],[ox] [first sample time]");\
 sep_add_doc_line("");\
 sep_add_doc_line("    tpow -float");\
 sep_add_doc_line("               power of time");\
 sep_add_doc_line("");\
 sep_add_doc_line("    panel -char*");\
 sep_add_doc_line("               [1]: (default) 1st panel is used for estimating tpow.");\
 sep_add_doc_line("               When panel=all, all panels are used.");\
 sep_add_doc_line("");\
 sep_add_doc_line("    tmin -float");\
 sep_add_doc_line("                                [o1]:   time of the first sample to be used in estimating tpow. ");\
 sep_add_doc_line("");\
 sep_add_doc_line("    tmax -float");\
 sep_add_doc_line("               [4.]: end of region used on time axis");\
 sep_add_doc_line("");\
 sep_add_doc_line("    ofdep -char*");\
 sep_add_doc_line("               [unknown]      unknown: automatic picking of first");\
 sep_add_doc_line("                 arrivals will be applied to compute the muting parameters");\
 sep_add_doc_line("               no:  data are offset-independent ");\
 sep_add_doc_line("               yes: data are offset-dependent ");\
 sep_add_doc_line("");\
 sep_add_doc_line("    v0- float");\
 sep_add_doc_line("               (km/s) apparent |dx/dt|. muting ");\
 sep_add_doc_line("                before first arrival time will ");\
 sep_add_doc_line("                be applied in estimating the ");\
 sep_add_doc_line("                medians. when no v0 is given");\
 sep_add_doc_line("                and ofdep is not 'no', automatic");\
 sep_add_doc_line("                picking will be applied ");\
 sep_add_doc_line("");\
 sep_add_doc_line("    nxmin - int");\
 sep_add_doc_line("               [1]: the first trace used for Tpow ");\
 sep_add_doc_line("");\
 sep_add_doc_line("    nxmax=nx");\
 sep_add_doc_line("               the last trace used. For 3-d data,");\
 sep_add_doc_line("               trace nxmin to trace nxmax are ");\
 sep_add_doc_line("               used on each plane if panel=all.");\
 sep_add_doc_line("");\
 sep_add_doc_line("    eps - float");\
 sep_add_doc_line("               [0.05]: (default) iteration of estimating dtpow stops");\
 sep_add_doc_line("               when abs(dtpow) < eps.");\
 sep_add_doc_line("");\
 sep_add_doc_line("    idt - integer");\
 sep_add_doc_line("               [2]: subsample rate (integer) of time for tpow");\
 sep_add_doc_line("               estimating");\
 sep_add_doc_line("");\
 sep_add_doc_line("    idx - integer");\
 sep_add_doc_line("               [1]: subsample rate (integer) of trace for tpow estimating");\
 sep_add_doc_line("");\
 sep_add_doc_line("    iwind - integer");\
 sep_add_doc_line("               [9]: window length used for calculating L-1 energy in autopicking");\
 sep_add_doc_line("");\
 sep_add_doc_line("    perc - float");\
 sep_add_doc_line("               [10.]: when L-1 norm of present window exceeds");\
 sep_add_doc_line("               that of the previous one by ");\
 sep_add_doc_line("               'perc', first arrival is picked ");\
 sep_add_doc_line("");\
 sep_add_doc_line("COMMENTS");\
 sep_add_doc_line("  METHOD");\
 sep_add_doc_line("      m02 * (.25*tmax)**(dtpow) = m24 * (.75*tmax)**(dtpow)");\
 sep_add_doc_line("      tpow = tpow + dtpow");\
 sep_add_doc_line("      where ");\
 sep_add_doc_line("        m02 --- median over (0, tmax/2)    of | data(t) | * t**tpow");\
 sep_add_doc_line("        m24 --- median over (tmax/2, tmax) of | data(t) | * t**tpow");\
 sep_add_doc_line("");\
 sep_add_doc_line("CATEGORY");\
 sep_add_doc_line("    seis/filter");\
 sep_add_doc_line("");\
 sep_add_doc_line("COMPILE LEVEL");\
 sep_add_doc_line("    DISTR");\
 sep_add_doc_line("");

#include <sep.main>
MAIN()
{
	int nt, it, nx, n3, idx=1, nxx, ntt, ix, ip, nxmin, nxmax, n3tpow;
	int ntmin, nxtest, iofdep=0;
	float *xx, *yy, *tgain, tpow, *xxpipe;
	float newtpow = 0.0, v0 = - 1000000., t0x0 = 0., dx;
	float dt,t0,x0,eps,tmax,tmin;
	float xmin, dxtpow, dttpow;
	int ipeak, iwind, idt=2,n;
	float perc, xsum, xsup2, xysum, ysum, *tfst;
	float xabs, a, b, den, per;
	float var0, var1, test2;

        sep_begin_prog();

	/* obtain parameters */
	if (!fetch("n1","d",&nt)) seperr ("n1= missing\n"); putch("n1","d",&nt);
	nx=1; fetch("n2","d",&nx); putch("n2","d",&nx);
	n3=1; fetch("n3","d",&n3); 
	n=1; fetch("n4","d",&n); n3=n3*n;
	n=1; fetch("n5","d",&n); n3=n3*n;
	n=1; fetch("n6","d",&n); n3=n3*n;
	n=1; fetch("n7","d",&n); n3=n3*n;
        

	if(!fetch("d1","f",&dt)) seperr ("dt= missing\n"); putch("d1","f",&dt);
	t0 = 0.; fetch("o1","f",&t0); putch("o1","f",&t0);
	dx=.025; fetch("d2","f",&dx); 
	x0 = 0.; fetch("o2","f",&x0); 
	if (dx < 0.) {
		dx = - dx ;
		x0 = - x0 ;    
    }

	putch("d2","f",&dx);
	putch("o2","f",&x0);

	/* read data of 1st plane into xxpipe used in case of piped input */
	xxpipe = (float *) alloc (nt*nx * sizeof(float) );
	make_unpipe("in");
	sseek("in",0,0);
	for (ix=0;ix<nx;ix++) sreed("in",xxpipe+ix*nt,nt*4);  
  
	if(getch("tpow","f",&tpow)) putch("Given-tpow= applied_tpow","f",&tpow);

	else { /* tpow not given, then estimate best tpow */
		getch("panel","s",panel); putch("panel for_tpow","s",panel); 
		eps = 0.05; getch("eps","f",&eps);
		if(getch("tmax","f",&tmax)) putch("tmax","f",&tmax);
		else tmax = 4.;
		nxmin=1;  getch("nxmin","d",&nxmin); putch("nxmin","d",&nxmin); 
		nxmax=nx; getch("nxmax","d",&nxmax); putch("nxmax","d",&nxmax); 
		if(getch("tmin","f",&tmin)) putch("tmin given","f",&tmin);
		else tmin = t0;
		if(getch("ofdep","s",ofdep)) putch("ofdep given","s",ofdep);
		if (getch("v0","f",&v0)) {
			putch("v0 given","f",&v0);  
			t0x0 = 0.;
			iofdep = 1;
		}

		/* auto-picking of the first arrival times for traces */
		if (((ofdep[0] == 'y') && (v0 == -1000000.)) || (ofdep[0] == 'u')) {

			/* obtain parameters for picking */
			iwind=9;getch("iwind","d",&iwind); 
			if (nt < 128) iwind = nt/16+1;
			putch("iwind","d",&iwind);
			perc=10; getch("perc","f",&perc); putch("perc","f",&perc);
			if (perc <= 3.333) seperr("perc too small");
			yy = (float *) alloc (nt*sizeof(float));
			tfst = (float *) alloc (nx * sizeof(float));
			for(ix=0;ix<nx;ix++) {  
				for (it=0;it<nt;it++)  yy[it] = xxpipe[ix*nt+it];

				/* auto-picking of the first arrival time for the trace */
				per = .3 * perc + .7 * perc * exp( - ix * .05) ;
				fstpick(yy,&nt,&ipeak,&iwind,&per);
				tfst[ix] = t0 + ipeak * dt;
			}
			if (nx <= 5) {
				t0x0 = 0.;
				for (ix=0;ix<nx;ix++) t0x0 += tfst[ix]/nx;  
				if (t0x0 > nt/2 * dt) t0x0 = 0.; 
				if (tmin < t0x0) tmin = t0x0;
				iofdep = 0;
				putch("ofdep","s","ignored");
			} else {  

				/* least-square fitting of the first arrival times with constrain*/
				xsup2 = 0.;
				xsum = 0.;
				xysum = 0.;
				ysum = 0.;
				nxx = 0;
				if (dx < 1. && dt < 0.1) /* km-sec units used */   
					var0 = fabs(dx/1.0);
				else if (dx>1. && dt<0.1) /* m-sec units used */ 
					var0 = fabs(dx/1000.);
				else seperr("specify your units in terms of km and sec.");

				/* least-square fitting with t = a + b * |x|, with constraints */
				for (ix=1;ix<nx-1;ix++) {
					var1 = tfst[ix] - tfst[ix-1];  
					xabs =  fabs( x0 + ix * dx );   
					test2 = xabs * var0 / dx ; 
					if (tfst[ix] != t0)  /* ipeak != 0 */

					/* avoid sharp changes in both tfst[x] and its derivative */
					if((fabs(var1) <= var0 && fabs(var1) > 0.) || 
						(tfst[ix] <= test2 * 1.1 && tfst[ix] >= test2 * .3)) {

						xsup2 +=  xabs * xabs ;   
						xysum += xabs * tfst[ix];  
						ysum += tfst[ix] ; 
						xsum += xabs;
						nxx += 1;
					}
				}
				den = nxx * xsup2 - xsum * xsum;

				/* den=0 means a vertival line! */ 
				/* # of reliable points must not be less one eighth of total # */ 
				if ( den != 0. && nxx > nx/8 ) {
					a = (ysum * xsup2 - xysum * xsum) / den ;
					b = ( nxx * xysum - xsum * ysum ) / den ;
				} else if (den == 0. || nxx <= nx/8)  {

					/* if fitting with constraints failed, then try 
						without constraint */
					for (ix = 0; ix < nx; ix++) {
						xabs =  fabs( x0 + ix * dx );   
						xysum += xabs * tfst[ix];  
						ysum += tfst[ix] ; 
						xsum += xabs;
					}
					den = nx * xsup2 - xsum * xsum;
					if (den == 0.) /* den=0 means a vertival line */ 
						seperr("apparent |dt/dx| is infinite, please specify v0");
					else { /* fitting without constraint */  
						a = (ysum * xsup2 - xysum * xsum) / den ;
						b = (nx * xysum - xsum * ysum) / den ;
					}
				}
				t0x0 = a;
				if (fabs(t0x0) > nt/2*dt) t0x0 = 0.; 
				if (fabs(x0) > fabs(x0+(nx-1)*dx)) xabs = fabs(x0);   
				else xabs = fabs(x0+(nx-1)*dx);
				if (xabs * b / dt > nt) 
					seperr("apparent |dt/dx| is too large, please specify v0");
				if (xabs * b / dt <= 8) {
					if (ofdep[0] == 'y') seperr("please specify your v0");
					else if ( ofdep[0] == 'u') {
						iofdep = 0;
						if (t0x0 > tmin) tmin = t0x0; 
						putch("ofdep detected","s","no");  
					}
				} else {
					iofdep = 1;
					v0 = 1./ b;
					putch("ofdep detected","s","yes");  
					putch("v0 detected","f",&v0);
				}
			}
		}
    
		if (panel[0] == 'f') n3tpow=1;
		if (panel[0] == 'a') n3tpow = n3;
  
		idt = 2; getch("idt","d",&idt); putch("idt","d",&idt);
		idx = 1; getch("idx","d",&idx); putch("idx","d",&idx);
		dttpow = idt * dt;   
		dxtpow = idx * dx;
		ntmin=(tmin-t0)/dt;
		if (ntmin < 0) ntmin = 0;
		ntt = (nt-ntmin)/idt; 

		/* window the data from tmin-tmax seconds once each idt samples*/
		if((t0 + nt*dt) > tmax) ntt = (tmax - tmin)/dttpow;
		xmin = x0 + (nxmin-1) * dx;
		putch("xmin for estimating_tpow","f",&xmin);
		nxx = (nxmax - nxmin + 1 )/idx; 

		/* maximum working space XXSZ */
		if (ntt*nxx*n3tpow*4 > XXSZ) {
			nxtest = XXSZ / (n3tpow*ntt*4);
			idx = nxx / nxtest;
			nxx = nxtest;
			dxtpow = idx * dx;
		}
		xx = (float *) alloc (ntt*nxx*n3tpow * sizeof(float));

		/* subsample 1st plane data if necessary */
		for (ix=0;ix<nxx;ix++)
			for (it=0;it<ntt;it++)
				xx[it+ix*ntt] = xxpipe[it*idt+(ix*idx+nxmin-1)*nt+ntmin];

		/* read more data when panel=all */
		if (n3tpow > 1) {
			for (ip=1;ip<n3tpow;ip++) {
				for (ix=0;ix<nxx;ix++) {
/*					sseek("in",(ip*nx+(ix*idx+nxmin-1))*nt*4+ntmin*4,0);*/
          sseek_block("in",nt*4,(ip*nx+(ix*idx+nxmin-1)),0);
          sseek("in",ntmin*4,1);
					for(it=0;it<ntt;it++) {
						sseek("in",idt*4,1);
/*
          reed(infd,xx+(ip*nxx+ix)*ntt+it,4);
*/
						sreed("in",xx+(ip*nxx+ix)*ntt+it,4);
					}
				}
			}
		}

		/* estimate tpow from xx[ntt,nxx] */
		esttpow(xx,&ntt,&nxx,&n3tpow,&dttpow,&tmin,&tpow,&iofdep,&v0,
			&t0x0,&eps,&xmin,&dxtpow);
		putch("estimated_tpow= applied_tpow","f",&tpow);
	}
	putch("For plotting: tpow","f",&newtpow);
	hclose();
	yy = (float *) alloc (nt* sizeof(float));
	tgain = (float *) alloc (nt* sizeof(float));
	for(it=0;it<nt;it++) {
		tgain[it] = pow((t0+(it+1)*dt), tpow); 
    }

	/* output gained data of 1st plane */
    for(ix=0;ix<nx;ix++)  {
		for(it=0;it<nt;it++) {
			yy[it] = xxpipe[it+ix*nt]*tgain[it];
        }
		srite("out",yy,nt*4);
	}

	/* output 3-d gained data */     
	sseek("in",0,0);  
	for(ip=1;ip<n3;ip++) {
		for(ix=0;ix<nx;ix++)  {
/*			sseek("in",(ix+ip*nx)*nt*4,0); */
			sseek_block("in",(ix+ip*nx),nt*4,0); 
			sreed ("in",yy,nt*4);
			for(it=0;it<nt;it++) {
				yy[it] *= tgain[it];
			}
			srite("out",yy,nt*4);
		}
	}

        sep_end_prog();
	return(0);
}      

/*********************************************************************
*                        Subroutine esttpow                          *
**********************************************************************
*          subroutine estimates tpow from the data xx[ntt,nxx]       *
**********************************************************************/

#if NeedFunctionPrototypes
_XFUNCPROTOBEGIN
void esttpow(float *xx,int *antt,int *anxx,int *an3,float *adttpow, float *atmin,
	float *atpow,int *aiofdp,float *av0,float *at0x0,float *aeps,float *axmin,
	float *adxtpw)
_XFUNCPROTOEND
#else
void esttpow(xx,antt,anxx,an3,adttpow,atmin,atpow,aiofdp,av0,at0x0,aeps,axmin,adxtpw)
	float *xx,*aeps, *av0, *adxtpw, *axmin, *at0x0;
	float *adttpow, *atmin, *atpow;
	int *antt, *anxx, *an3;
	int *aiofdp;
#endif

{
	int it,ix, n, n02, n24;
	int ntt,nxx, ntt2, n3, nt0ix;
	int count1, count2, ip; 
	int iofdep, ist, itn;
	int *itnum1, *itnum2, *it00x, *it02x;
	float eps,tpow,dtpow,tmin,v0, t0x0;
	float m02,m24,dttpow;
	float dxtpw, xmin, tmp;
	float *work1, *work2;
	float *dtpw;
	float percent;

	eps = *aeps;
	if (*aeps == 0.) eps = 0.05;
	ntt = *antt; nxx = *anxx; n3 = *an3;
	iofdep = *aiofdp; v0 = *av0; t0x0 = *at0x0; 
	dttpow = *adttpow; tmin = *atmin;  
	dxtpw = *adxtpw; xmin = *axmin;
	ntt2= ntt/2;
	n = ntt2*nxx*n3;

	/* allocate working space */
	dtpw = (float *) malloc(ntt*sizeof(float));
	itnum1 = (int *) malloc ( nxx * sizeof(int));
	itnum2 = (int *) malloc (nxx * sizeof(int));
	it00x = (int *) malloc (nxx * sizeof(int));
	it02x = (int *) malloc (nxx * sizeof(int));

	count1 = 0;
	count2 = 0;
	if (iofdep == 0) {
		work1 = (float *) malloc(n*sizeof(float));
		work2 = (float *) malloc(n*sizeof(float));
		n02 = ntt2 * nxx * n3;
		n24 = ntt2 * nxx * n3;
		for (ix=0;ix<nxx*n3;ix++) {
			for (it=0;it<ntt2;it++) {
				work1[it+ix*ntt2] = fabs(xx[ix*ntt+it]);
				work2[it+ix*ntt2] = fabs(xx[ix*ntt+it+ntt2]);
			}
		}
	} else {
		n02 = 0; n24 = 0; 
		for (ix=0;ix<nxx;ix++) {
			itnum1[ix] = 0;
			itnum2[ix] = 0;
			it00x[ix] = 0;
			it02x[ix] = ntt2;
		}
		for (ix=0;ix<nxx;ix++) {
			tmp = fabs(ix*dxtpw+xmin)/v0 + t0x0 - tmin;   
			if (tmp  < 0.)  tmp = 0.;
			nt0ix = tmp / dttpow;
			if (ntt2 > nt0ix) {
				itnum1[ix] = ntt2 - nt0ix;
				itnum2[ix] = ntt2;
				it00x[ix] = nt0ix;
				it02x[ix] = ntt2;
				n02 += itnum1[ix];  
				n24 += itnum2[ix];  
			} else if (nt0ix >= ntt2 && nt0ix < ntt2*2)  {
				itnum1[ix] = 0;
				itnum2[ix] = 2*ntt2 - nt0ix;
				it00x[ix] = nt0ix;
				it02x[ix] = nt0ix;
				n24 += itnum2[ix];  
			}
		}
		n24 *= n3;
		n02 *= n3;
		work1 = (float *) malloc(n02*sizeof(float));
		work2 = (float *) malloc(n24*sizeof(float));
		count1 = 0 ; count2 = 0;      
		for (ix=0;ix<nxx;ix++) {
			itn = itnum1[ix] ;
			ist = it00x[ix] ;
			for (ip=0;ip<n3;ip++) {
				for (it=0;it<itn;it++) 
					work1[it+count1] = fabs(xx[it+ist+(ip*nxx+ix)*ntt]);
				count1 += itn;
			}
			itn = itnum2[ix];
			ist = it02x[ix];
			for (ip=0;ip<n3;ip++) {
				for (it=0;it<itn;it++)
					work2[it+count2] = fabs( xx[it+ist+(ip*nxx+ix)*ntt]);
				count2 += itn;
			}
		}
	}

	tpow = 2.;
	dtpow = 2.;
	percent = 50.;
	while (fabs( dtpow) > eps) {
		for(it=0;it<ntt;it++)
			dtpw[it] = pow(it*dttpow+tmin, dtpow);
		if (iofdep == 0) {
			for (ix=0;ix<nxx*n3;ix++) {
				for (it=0;it<ntt2;it++) {
					work1[it+ix*ntt2] *= dtpw[it];
					work2[it+ix*ntt2] *= dtpw[it+ntt2];
				}
			}
		} else {
			count1 = 0;
			count2 = 0;
			for (ix=0;ix<nxx;ix++) {
				itn = itnum1[ix] ;
				ist = it00x[ix] ;
				for (ip=0;ip<n3;ip++) {
					for (it=0;it<itn;it++) 
						work1[it+count1] *= dtpw[it+ist];
					count1 += itn;
				}
				itn = itnum2[ix] ;
				ist = it02x[ix] ;
				for (ip=0;ip<n3;ip++) {
					for (it=0;it<itn;it++) work2[it+count2] *= dtpw[it+ist];
					count2 += itn;
				}
			}
		}

		/* find the medians */
		m02 = cent(percent,work1,n02);
		m24 = cent(percent,work2,n24);

		/* find DELTA tpow */
		if (m24 != 0.) dtpow = log(m02/m24) / log(3.);  
		else dtpow = 0.;
		if (fabs(dtpow) > 10.) dtpow = 10. * dtpow / fabs(dtpow) ; 
		tpow += dtpow;
	}
	*atpow = tpow;

	/* clean up */
	free (work1);
	free (work2);
	free (xx);
}


/*********************************************************************
*                        Subroutine cent                             *
**********************************************************************
*                     computes percentiles                           *
**********************************************************************
*      p - percentile <0.,99.99999999>                               *
*      x - data                                                      *
*      n - vector raslength                                          *
**********************************************************************
*      based on Canales, SEP-10                                      *
**********************************************************************/

#if NeedFunctionPrototypes
_XFUNCPROTOBEGIN
float cent (float p,float *x,int n)
_XFUNCPROTOEND
#else
float cent (p,x,n)
	int n;
	float *x, p;
#endif

{
	int q;
	register float *i, *j, ak;
	float *low, *hi, buf, *k;

	if (p>99.99999999) p = 99.99999999;
	if (p<0.) p = 0.; q = (p*n)/100.;
	for (low=x, hi=x+n-1, k=x+q; low<hi;) {
		ak = *k;
		i = low; j = hi;
		do {
			while (*i < ak) i++;     
			while (*j > ak) j--;     
			if (i<=j) {
				buf = *i;
				*i++ = *j;
				*j-- = buf;
			}
		} while (i<=j);
		if (j<k) low = i; if (k<i) hi = j;
	}
	return (*k);
}

/*********************************************************************
*                        Subroutine fstpick                          *
**********************************************************************
*                   Auto-picking of first arrivals                   *
**********************************************************************/

#if NeedFunctionPrototypes
_XFUNCPROTOBEGIN
void fstpick(float *x,int *ant,int *aitpeak,int *alwind,float *aperc)
_XFUNCPROTOEND
#else
void fstpick(x,ant,aitpeak,alwind,aperc)
	int *alwind, *aitpeak, *ant;
	float *x,*aperc;
#endif

{
	int itpeak,nt,lwind,iwind,it,iw,it0;
	float *y,perc,percc,mean0=0.,mean1=0.,peak;

	nt = *ant;
	*aitpeak = 0;
	itpeak = 0;
	lwind = *alwind;  
	if(lwind == 0 && nt > 128) lwind = 8 + 1;
	else if (nt < 128) lwind = nt/16 + 1;
	if(*aperc == 0.) perc=10.;
	else perc = *aperc; 
	if (perc <= 1.) seperr("perc has to be greater than 1");
	y = (float *) alloc(nt*sizeof(float));

	for (it=0;it<nt;it++) {
		if( x[it] < 0. ) y[it] = - x[it];
		else y[it] = x[it]; 
    }

 	/* if previous picking failed, reduce length of window and try again */
	for (iwind = lwind; iwind > 1 ; iwind--) {

		/* compute l1 norm of amplitudes over the first window */
		for (iw=0; iw<iwind; iw++) mean0 += y[iw];
		it = iwind; it0=0;

		/* if previous picking failed, reduce percc and try again */
		for(percc=perc; percc > 1.; percc -= .5) {
			while (it < nt-iwind) {
				mean1 = 0.;
				for (iw=0; iw<iwind; iw++) mean1 += y[it+iw];

				/* (mean1 < mean0*percc*1.e+3) avoids too sharp change
					in the L1 norm due to "zeroes" before the first small
					value; for picking spiky first break (synthetic data),
					remove this constraint */
				if (mean1 > mean0 * percc && mean1 < mean0*percc*1.e+3) {
					it0 = it;
					goto pick; /* peak determined within the window*/
				} else {
					mean0 = mean1;
					it += iwind;
				}
			}  
		}
	}

	/* determine the first peak within the window */
	pick: 
	if (it0 != 0) {
		itpeak = it0;
		it = 1;
		peak = y[it0];  
		while (it < iwind) {

			/* peak occurs where amplitude becomes smaller than the previous one  */
			if (peak > y[it0+it]) {
				itpeak += it;
				break;
			} else {
				peak = y[it0+it]; 
				it += 1; 
			}
		}   
	}

	*aitpeak = itpeak;
	free(y);
}
