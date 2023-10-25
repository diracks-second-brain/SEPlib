/*$

=head1 NAME

Ft3d - incore Fourier tranform

=head1 SYNOPSIS

Ft3d < in.H sign{1,2,3}=0. center{1,2,3}=0. > out.H
 

=head1 INPUT PARAMETERS

=over 4

=item  sign{1,2,3} -integer 

	[0]: means omit Fourier transform
	 +1,-1: sign of sqrt(-1) on dimension {1,2,3}

=item  center{1,2,3}-integer 

	[0]: don't multiply
	 +1: multiply by 1,-1,1,-1,... before transform
         (centers axis in transform space)
	 -1: multiply by 1,-1,1,-1,... after transform
        (allows centered transforms to be inverted)

=item  n1,n2,n3 -integer     

       standard seplib params
       dimensions transformed must be of size 2**N
       scaling is 1/sqrt(n) all direction

=item  o1,o2,o3 -real        

        standard seplib params

=item  d1,d2,d3 -real        

       standard seplib params

=item	maxsize - int         

      [20] Amount of memory to use in megabytes

=back
 
=head1 DESCRIPTION 

Ft3d - in core 1,2, or 3 dimensional Fourier transform

=head1 COMMENTS

Assumes in= contains COMPLEX-VALUED data.

=head1 SEE ALSO

L<Cfft>

=head1 CATEGORY

B<seis/filter>

=cut
>*/

/*
 
  KEYWORDS      1-d  2-d  3-d Fourier-transform

  WHERE
        ./cube/seis/Ft3d.c

copyright (c) 1991 Stanford University 
*/
/*
Modified  10/9/86  stew    added putch's for fft parameters.
			   program now hetch's previous value if any,
			   negating value of signs
	  7/27/87  steve   moved all putch's and getch's here from ft3d.r
			   so that latter could be called by other
			   programs.
	  3/2/88   steve   explicitly initalize center1,2,3 to 0 at stew's
			   suggestion.
	  8/9/89   harlan  update o1,o2, o3, and labels on output
	  Feb 24 1992 joe  double Fft puts labels back to original
5B
	  11-3-92  martin  changed alloc(sizeof()) and sreed/srite sizeof(xdr)
	  11-11-92  dave   do 2-D panels in turn if no 3-axis transform
	  6-11-94  stew    fixed null termination of label strings
	  6-22-94  stew    use tetch to avoid horrendous precision loss
                           in axis increments due to repeated ascii-binary
                           conversions.
		8-1-99 Bob      Straight C code using cefft, maxsize, rewrite interior

*/
/*
 * Keyword: 1-d  2-d  3-d Fourier-transform Ft3d
 */


#include <sepmathf.h>

/*
   sets up infd, outfd, instream, outstream, doc (SOURCE), pi, and 
*/
#include <stdio.h>
#include <math.h>
#include <seplib.h>
#include <string.h>
#include <sepfft.h>


#if NeedFunctionPrototypes
_XFUNCPROTOBEGIN
int fft_it(complex *trace,float sign,float center ,int n);
_XFUNCPROTOEND
#else
int ft3d_it();
int fft_it();
#endif

complex  *data;
#define MY_SEP_DOC \
 sep_add_doc_line("NAME");\
 sep_add_doc_line("    Ft3d - incore Fourier tranform");\
 sep_add_doc_line("");\
 sep_add_doc_line("SYNOPSIS");\
 sep_add_doc_line("    Ft3d < in.H sign{1,2,3}=0. center{1,2,3}=0. > out.H");\
 sep_add_doc_line("");\
 sep_add_doc_line("INPUT PARAMETERS");\
 sep_add_doc_line("    sign{1,2,3} -integer");\
 sep_add_doc_line("                [0]: means omit Fourier transform");\
 sep_add_doc_line("                 +1,-1: sign of sqrt(-1) on dimension {1,2,3}");\
 sep_add_doc_line("");\
 sep_add_doc_line("    center{1,2,3}-integer");\
 sep_add_doc_line("                [0]: don't multiply");\
 sep_add_doc_line("                 +1: multiply by 1,-1,1,-1,... before transform");\
 sep_add_doc_line("                 (centers axis in transform space)");\
 sep_add_doc_line("                 -1: multiply by 1,-1,1,-1,... after transform");\
 sep_add_doc_line("                (allows centered transforms to be inverted)");\
 sep_add_doc_line("");\
 sep_add_doc_line("    n1,n2,n3 -integer");\
 sep_add_doc_line("               standard seplib params");\
 sep_add_doc_line("               dimensions transformed must be of size 2**N");\
 sep_add_doc_line("               scaling is 1/sqrt(n) all direction");\
 sep_add_doc_line("");\
 sep_add_doc_line("    o1,o2,o3 -real");\
 sep_add_doc_line("                standard seplib params");\
 sep_add_doc_line("");\
 sep_add_doc_line("    d1,d2,d3 -real");\
 sep_add_doc_line("               standard seplib params");\
 sep_add_doc_line("");\
 sep_add_doc_line("    maxsize - int");\
 sep_add_doc_line("              [20] Amount of memory to use in megabytes");\
 sep_add_doc_line("");\
 sep_add_doc_line("DESCRIPTION");\
 sep_add_doc_line("    Ft3d - in core 1,2, or 3 dimensional Fourier transform");\
 sep_add_doc_line("");\
 sep_add_doc_line("COMMENTS");\
 sep_add_doc_line("    Assumes in= contains COMPLEX-VALUED data.");\
 sep_add_doc_line("");\
 sep_add_doc_line("SEE ALSO");\
 sep_add_doc_line("    Cfft");\
 sep_add_doc_line("");\
 sep_add_doc_line("CATEGORY");\
 sep_add_doc_line("    seis/filter");\
 sep_add_doc_line("");
#include <sep.main>

MAIN()
{
	int n1=0, n2=0, n3=0, esize,nbyte,i,num;
	float sign1=0.,sign2=0.,sign3=0., center1=0.,center2=0.,center3=0.;
	float d1=1., d2=1., d3=1., o1=0., o2=0., o3=0.;
	int   d1fl=0, d2fl=0, d3fl=0;
	char labelin[80],labelout[80],temp_ch[1024];
	int nbig,maxsize,nlittle,tempi,nmax,i4,i3,i2,i1;
	complex *array,*trace;

        sep_begin_prog();

	/* get input parameters and set defaults */
	if(!fetch("n1","d",&n1))
		if(!fetch("nt","d",&n1))  seperr("Can't find n1=%d\n",n1);
	if(!fetch("n2","d",&n2))
		if(!fetch("nx","d",&n2))  n2=1;
	if(!fetch("n3","d",&n3))
		if(!fetch("np","d",&n3))  n3=1;
	if(!fetch("esize","d",&esize))
		seperr("esize not found\n");
	if( esize != 8 ) seperr(" esize must be 8\n");

	/* get aditional parameters */
	hetch("d1","f",&d1);
	hetch("d2","f",&d2);
	hetch("d3","f",&d3);

	/* get transform signs (signs of sqrt(-1)) */
	if(getch("sign1","f",&sign1) == 0) {
		if(hetch("sign1","f",&sign1) == 0) sign1 = 0.;
		else sign1 = -sign1;
	}
	if(getch("sign2","f",&sign2) == 0) {
		if(hetch("sign2","f",&sign2) == 0) sign2 = 0.;
		else sign2 = -sign2;
	}
	if(getch("sign3","f",&sign3) == 0) {
		if(hetch("sign3","f",&sign3) == 0) sign3 = 0.;
		else sign3 = -sign3;
	}

	/* put signs back to history file */
	putch("sign1","f",&sign1);
	putch("sign2","f",&sign2);
	putch("sign3","f",&sign3);

	/* if necessary, put center options back to history file */
	if(getch("center1","f",&center1) != 0)
		putch("center1","f",&center1);
	if(getch("center2","f",&center2) != 0)
		putch("center2","f",&center2);
	if(getch("center3","f",&center3) != 0) 
		putch("center3","f",&center3);

	/* hadle labels in output data according to the transform options */
	if(!fetch("label1","s",labelin)) sprintf(labelin,"%s","sample");
	if(sign1!=0) {
		if (strlen(labelin) >= 4 && strncmp(labelin,"1/[",3) == 0
		 && labelin[strlen(labelin)-1] == ']') {
		    strncpy(labelout, &labelin[3], strlen(labelin) - 4);
		    labelout[strlen(labelin)-4] = '\0';
		    d1fl = 1;
		} else {
		    sprintf(labelout,"1/[%s]",labelin);
		}
		putch("label1","s",labelout);
	}
	if(!fetch("label2","s",labelin)) sprintf(labelin,"%s","sample");
	if(sign2!=0) {
		if (strlen(labelin) >= 4 && strncmp(labelin,"1/[",3) == 0
		 && labelin[strlen(labelin)-1] == ']') {
		    strncpy(labelout, &labelin[3], strlen(labelin) - 4);
		    labelout[strlen(labelin)-4] = '\0';
		    d2fl = 1;
		} else {
		    sprintf(labelout,"1/[%s]",labelin);
		}
		putch("label2","s",labelout);
	}
	if(!fetch("label3","s",labelin)) sprintf(labelin,"%s","sample");
	if(sign3!=0) {
		if (strlen(labelin) >= 4 && strncmp(labelin,"1/[",3) == 0
		 && labelin[strlen(labelin)-1] == ']') {
		    strncpy(labelout, &labelin[3], strlen(labelin) - 4);
		    labelout[strlen(labelin)-4] = '\0';
		    d3fl = 1;
		} else {
		    sprintf(labelout,"1/[%s]",labelin);
		}
		putch("label3","s",labelout);
	}

	/* put the right sampling intervals in the output data */
	if(sign1!=0) {
		d1 = 1. / (n1*d1);
		if(d1fl) tetch("d1","f",&d1);
		putch("d1","f",&d1);
	}
	if(sign2!=0) {
		d2 = 1. / (n2*d2);
		if(d2fl) tetch("d2","f",&d2);
		putch("d2","f",&d2);
	}
	if(sign3!=0) {
		d3 = 1. / (n3*d3);
		if(d3fl) tetch("d3","f",&d3);
		putch("d3","f",&d3);
	}

	/* put the right axis origins in the output data */
	if(center1!=0) {
		fetch("o1","f",&o1); 
		o1 = - d1*n1/2;
		putch("o1","f",&o1); 
	}
	if(center2!=0) {
		fetch("o2","f",&o2); 
		o2 = - d2*n2/2;
		putch("o2","f",&o2); 
		}
	if(center3!=0) {
		fetch("o3","f",&o3); 
		o3 = - d3*n3/2;
		putch("o3","f",&o3); 
	}


/***********************/
/* Begining of change **/
/*    8-1-99           */
/***********************/
	if(0==getch("maxsize","d",&maxsize)) maxsize=20;
	maxsize=maxsize*1000000;
	putch("#maxsize in bytes","d",&maxsize);
	hclose();

	nbig=1;
	i=4;sprintf(temp_ch,"n%d",i);
	while(1==hetch(temp_ch,"d",&tempi)){ 
		nbig=nbig*tempi;i++;sprintf(temp_ch,"n%d",i);
	}

	/*figure out what the smallest chunck we can work on is */
	if(sign3!=0.){ 
		nbig=nbig; nlittle=n1*n2*n3;
	} else if(sign2!=0.) {
		if(n1*n2*n3>maxsize/8) {
			nbig=nbig*n3; 
			n3=1; 
			nlittle=n1*n2;
		} else{ 
			nlittle=n1*n2*n3;
		}
	} else {
		if(n1*n2*n3< maxsize/8) {
			nlittle=n1*n2*n3;
		} else if(n1*n2 < maxsize/8){ 
			nbig=nbig*n3; n3=1;nlittle=n1*n2;
		} else { 
			nbig=nbig*n2*n3; 
			n3=1; 
			n2=1; nlittle=n1;
		}
	}

	/* check for minimum memory size to hold the entire dataset */
	if(nlittle * 8 > maxsize) 
	seperr("I can't hold enough of the dataset in memory maxisze needs to be %d\n",
			nlittle*8);

	/* check to see if transform axes are a power of 2 */
	nmax=1;
	if(sign1!=0.) {
		tempi=2;
		nmax=MAX(nmax,n1);
		while(tempi < n1) tempi=tempi*2;
		if(tempi!=n1)seperr("n1=%d is not a power of 2\n",n1);
	}
	if(sign2!=0.) {
		tempi=2;
		nmax=MAX(nmax,n2);
		while(tempi < n2) tempi=tempi*2;
		if(tempi!=n2) seperr("n2=%d is not a power of 2\n",n2);
	}
	if(sign3!=0.) {
		tempi=2;
		nmax=MAX(nmax,n3);
		while(tempi < n3) tempi=tempi*2;
		if(tempi!=n3) seperr("n3=%d is not a power of 2\n",n3);
	}

	/*malloc memory */
	array=(complex*)alloc(sizeof(complex)*nlittle);
	trace=(complex*)alloc(sizeof(complex)*nmax);

	/*loop over non transform axes */
	for(i4=0; i4 < nbig; i4++) { 
		if(nlittle*8!=sreed("in",array,nlittle*8))
			seperr("trouble reading in data \n");

		if(sign3!=0){ /*do the transform over the third axes */

			/*loop over first two axes copy too trace */
			for(i2=0;i2<n2;i2++){
				for(i1=0;i1<n1;i1++){
					for(i3=0; i3<n3;i3++) { 
						trace[i3].re=array[i1+i2*n1+i3*n1*n2].re;
						trace[i3].im=array[i1+i2*n1+i3*n1*n2].im;
					}
					if(0!=fft_it(trace,sign3,center3,n3))
						seperr("trouble ffting the trace \n");
					for(i3=0; i3<n3;i3++) { 
						array[i1+i2*n1+i3*n1*n2].re=trace[i3].re;
						array[i1+i2*n1+i3*n1*n2].im=trace[i3].im;
					}
				}
			}
		}
		if(sign2!=0) {
			for(i3=0;i3<n3;i3++) {
				for(i1=0;i1<n1;i1++) {
					for(i2=0; i2<n2;i2++) { 
						trace[i2].re=array[i1+i2*n1+i3*n1*n2].re;
						trace[i2].im=array[i1+i2*n1+i3*n1*n2].im;
					}
					if(0!=fft_it(trace,sign2,center2,n2))
						seperr("trouble ffting the trace \n");
					for(i2=0; i2<n2;i2++) { 
						array[i1+i2*n1+i3*n1*n2].re=trace[i2].re;
						array[i1+i2*n1+i3*n1*n2].im=trace[i2].im;
					}
				}
			}
		}
		if(sign1!=0) {
			for(i3=0;i3<n3;i3++){
				for(i2=0;i2<n2;i2++){
					for(i1=0; i1<n1;i1++) { 
						trace[i1].re=array[i1+i2*n1+i3*n1*n2].re;
						trace[i1].im=array[i1+i2*n1+i3*n1*n2].im;
					}
					if(0!=fft_it(trace,sign1,center1,n1))
						seperr("trouble ffting the trace \n");
					for(i1=0; i1<n1;i1++) { 
						array[i1+i2*n1+i3*n1*n2].re=trace[i1].re;
						array[i1+i2*n1+i3*n1*n2].im=trace[i1].im;
					}
				}
			}
		}
		if(nlittle*8!=srite("out",array,nlittle*8))
			seperr("trouble writing out data \n");
	}
        sep_end_prog();
	return(0);
}
	
#if NeedFunctionPrototypes
_XFUNCPROTOBEGIN
int fft_it(complex *trace,float sign,float center ,int n)
_XFUNCPROTOEND
#else
int fft_it(trace,sign,center,n)
complex *trace;float sign;float center ;int n;
#endif
{
	int i;

	if(center > 0.0) {
		for(i=1; i < n; i=i+2){
			trace[i].re = - trace[i].re;
			trace[i].im = - trace[i].im;
		}
	}
	i=cefft(trace,n,sign,1./sqrt(1.*n));
	if(center<0.0){
		for(i=1; i < n; i=i+2){
			trace[i].re = - trace[i].re;
			trace[i].im = - trace[i].im;
		}
	}
	return(0);
}
/*  $Id: Ft3d.csep,v 1.2 2004/07/08 18:15:32 bob Exp $ */
/*  $Id: Ft3d.csep,v 1.2 2004/07/08 18:15:32 bob Exp $ */
