/*$

=head1 NAME

Energy  - Calculate energy in running windows

=head1 SYNOPSIS

    Energy <in.h lwind=21 j1=11 normalize=no average=yes verbose=no > out.h
 
=head1 INPUT PARAMETERS

=over 4

=item lwind   -  integer 

      [21]:  number of points in smoothing window

=item j1      -  integer 

      [11]:  increment between window centers; 
      equal to subsampling factor; j1=1 is no subsampling.

=item normalize-char*    

      [no]:  convert all energy values to range 
      of 0.0 to 1.0 .  (normalizes each panel separately)

=item average - char*    

      [yes]:  normalizes energy by length of window

=item verbose - char*    

      [no]: gets more loquacious if =yes

=item n1,n2,n3- int      

      standard seplib pars

=item o1,d1   - real     

      standard seplib pars

=back

=head1 DESCRIPTION

Calculate energy in running windows along fast axis of data
 
=head1 EXAMPLE

    Energy < in.H lwind=21 j1=11 normalize=no average=yes verbose=no > out.h
	Calculates the trace energy in running windowis with 11-sample distance
	between window centers. The smoothing window is 21 samples long. Normalize
	energy by window length. No cross-window normalization is applied.

=head1 CATEGORY

B<seis/filter>

=cut

>*/
/*
 
 
  KEYWORDS energy power semblance velocity-analysis contour

  WHERE
        ./cube/seis/Energy.c

*/ 
/*
#
written February 1986 by Paul Fowler 
July 1986 updated real vector calls in energy.f to match new calling
          conventions in rveclib -Paul Fowler
July 1986 added normalization by length of window -Paul Fowler
October 1986 corrected calculation of n1new -Paul Fowler
*/
/* Keywords : energy power semblance velocity-analysis contour
 */
#include<sepfilter.h>
#define MY_SEP_DOC \
 sep_add_doc_line("NAME");\
 sep_add_doc_line("    Energy - Calculate energy in running windows");\
 sep_add_doc_line("");\
 sep_add_doc_line("SYNOPSIS");\
 sep_add_doc_line("        Energy <in.h lwind=21 j1=11 normalize=no average=yes verbose=no > out.h");\
 sep_add_doc_line("");\
 sep_add_doc_line("INPUT PARAMETERS");\
 sep_add_doc_line("    lwind - integer");\
 sep_add_doc_line("              [21]:  number of points in smoothing window");\
 sep_add_doc_line("");\
 sep_add_doc_line("    j1 - integer");\
 sep_add_doc_line("              [11]:  increment between window centers; ");\
 sep_add_doc_line("              equal to subsampling factor; j1=1 is no subsampling.");\
 sep_add_doc_line("");\
 sep_add_doc_line("    normalize-char*");\
 sep_add_doc_line("              [no]:  convert all energy values to range ");\
 sep_add_doc_line("              of 0.0 to 1.0 .  (normalizes each panel separately)");\
 sep_add_doc_line("");\
 sep_add_doc_line("    average - char*");\
 sep_add_doc_line("              [yes]:  normalizes energy by length of window");\
 sep_add_doc_line("");\
 sep_add_doc_line("    verbose - char*");\
 sep_add_doc_line("              [no]: gets more loquacious if =yes");\
 sep_add_doc_line("");\
 sep_add_doc_line("    n1,n2,n3- int");\
 sep_add_doc_line("              standard seplib pars");\
 sep_add_doc_line("");\
 sep_add_doc_line("    o1,d1 - real");\
 sep_add_doc_line("              standard seplib pars");\
 sep_add_doc_line("");\
 sep_add_doc_line("DESCRIPTION");\
 sep_add_doc_line("    Calculate energy in running windows along fast axis of data");\
 sep_add_doc_line("");\
 sep_add_doc_line("EXAMPLE");\
 sep_add_doc_line("        Energy < in.H lwind=21 j1=11 normalize=no average=yes verbose=no > out.h");\
 sep_add_doc_line("            Calculates the trace energy in running windowis with 11-sample distance");\
 sep_add_doc_line("            between window centers. The smoothing window is 21 samples long. Normalize");\
 sep_add_doc_line("            energy by window length. No cross-window normalization is applied.");\
 sep_add_doc_line("");\
 sep_add_doc_line("CATEGORY");\
 sep_add_doc_line("    seis/filter");\
 sep_add_doc_line("");
#include <sep.main>
/*
 * sets up infd, outfd, instream, outstream, doc (SOURCE), pi, and 
 * #include <stdio.h>
 * #include <math.h>
 * #include <seplib.h>
 */


MAIN()
{
	int ave;         /* flag for normalizing by window length */
	int j1;          /* fast axis subsampling factor	*/
	int lwind;       /* length of smoothing window */
	int norm;        /* flag for normalizing to [0,1] range */
	int n1;          /* number of fast axis (time) points */
	int n1new;       /* output number of fast axis (time) points	*/
	int n2;          /* number of intermediate axis points */
	int n3;          /* number of slow axis points */
	float d1;        /* fast axis sampling interval */
	float o1;        /* fast axis origin	*/
	float *data;     /* data array */
	float *outdata;  /* output data vector */
         sep_begin_prog();

	/* Input parameters from data history header and command line */
	getparameters(&n1, &n2, &n3, &d1, &o1, &lwind, &j1, &n1new, &norm, &ave);

	/* Close data header file */
	hclose();

	/* Allocate space for work array */
	data = (float *) alloc( n1 * n2 * sizeof(float) );
	outdata = (float *) alloc( n1new * n2 * sizeof(float) );

	/* perform energy calculation  */
	if(0!=energy("in","out",data,outdata,n1,n2,n3,lwind,j1,n1new,norm,ave)) 
		seperr("trouble calling energy \n");

        sep_end_prog();
	return(0);
}

/**********************************************************************
*                   Subroutine getparameters                          *
***********************************************************************
*  getparameters: gets values for all parameters from data processing *
*  history file or from operating system command line for program     *
*  Energy                                                             *
*  Note that all arguments are passed as pointers.                    *
**********************************************************************/
getparameters(pn1, pn2, pn3, pd1, po1, plwind, pj1, pn1new, pnorm, pave)

int *pave;      /* flag for normalizing by window length */
int *pnorm;     /* flag for normalizing range of values	*/
int *pn1;       /* number of fast axis points */
int *pn2;       /* number of intermediate axis points */
int *pn3;       /* number of slow axis points */
int *pn1new;    /* number of output fast axis points */
float *pd1;     /* fast axis sampling interval */
float *po1;     /* fast axis origin	*/
int *plwind;    /* length of smoothing window */
int *pj1;       /* subsampling factor */

{
	int hlwind;          /* half length of window */
	int n1new;           /* number of output fast axis points */
	int vflag;           /* flag for verbose option */
	float d1new;         /* output fast axis sampling interval */
	float o1new;         /* output fast axis origin */
	char average[20];    /* normalization by window length */
	char normalize[20];  /* normalization of values */
	char verbose[20];    /* verbose option */

	/* Check for verbosity desired  */
	strcpy(verbose,"no");
	vflag = 0;
	getch("verbose","s",verbose);
	if((verbose[0] == 'y') || (verbose[0] == 'Y')) {
		vflag = 1;
		putch("#verbose","s",verbose);
	}

	/* Determine description of input data axes */
	if(!fetch("n1","d",pn1)) {
		seperr("Can't find n1 on history or command line\n");
	} 
	if(!fetch("d1","f",pd1)) {
		seperr("Can't find d1 on history or command line\n");
	} 
	if(!fetch("o1","f",po1)) {
		seperr("Can't find o1 on history or command line\n");
	}    

	if(!fetch("n2","d",pn2)) {
		seperr("Can't find n2 on history or command line\n");
	} 

	(*pn3) = 1;
	fetch("n3","d",pn3);

	if(vflag) {
		puthead("\t\tFound on input: \n");
		putch("n1","d",pn1);
		putch("d1","f",pd1);
		putch("o1","f",po1);
		putch("n2","d",pn2);
		putch("n3","d",pn3);
	}
   
	if((*pn1) <= 0) {
		seperr("n1 > 0 required on input. \n");
	}
	if((*pn2) <= 0) {
		seperr("n2 > 0 required on input. \n");
	}
	if( (*pn3) <= 0) {
		seperr("n3 > 0 required on input. \n");
	}
	if((*pd1) <= 0.0) {
		seperr("d1 > 0.0 required on input. \n");
	}

	/* Determine length of window to be used */
	*plwind = 21;
	fetch("lwind","d",plwind);
	putch("lwind","d",plwind);
	if(*(plwind) < 1) {
		seperr("lwind > 0 required\n");
	} else if ((*plwind) %2 != 1) {
		(*plwind) +=1;
		puthead("\t\tlwind must be odd; larger value assumed.\n");
		putch("lwind","d",plwind);
	} else if(*(plwind) > (*pn1)) {
		seperr("lwind <= n1 required\n");
	}

	/* Determine subsampling rate for window centers */
	hlwind = (*plwind)/2;
	*pj1 = hlwind + 1;
	fetch("j1","d",pj1);
	putch("j1","d",pj1);
	if((*pj1) < 1) {
		seperr("pj1 >= 1 required\n");
	} else if((*pj1) > ((*pn1)/2 )) {
		seperr("j1 too big\n");
	}

	/* Write new data dimensions to history file  */
	o1new = (*po1) + (*pd1) * hlwind;
	n1new = 1 + ((*pn1) - 2*hlwind - 1)/(*pj1);
	d1new = (*pd1) * (*pj1);
	*pn1new = n1new;

	putch("n1","d",pn1new);
	putch("d1","f",&d1new);
	putch("o1","f",&o1new);

	strcpy(normalize,"no");
	fetch("normalize","s",normalize);
	putch("normalize","s",normalize);
	if(normalize[0] == 'n' || normalize[0] == 'N') {
		*pnorm = 0;
	} else {
		*pnorm = 1;
	}

	strcpy(average,"yes");
	fetch("average","s",average);
	putch("average","s",average);
	if(average[0] == 'n' || average[0] == 'N') {
		*pave = 0;
	} else {
		*pave = 1;
	}
	return(0);
}
