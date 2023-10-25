/* 
*/
/*
Author: Steve Cole (SEP)
Revised: Steve Cole (SEP)
  Added options to output balance factors, or read in balance factors
  from aux input and apply them. Useful if you want to compute factors
  from data in one form (such as amplitude spectra over some frequency
  range) and apply them to the raw data.
Revised: D. Nichols and R. Abma (SEP)
        Corrected auxillary file input check.
Martin Karrenbach 11-3-92  changed alloc(sizeof()) and sreed/srite sizeof(xdr)
Revised: Morgan Brown (23Jan2003) - Removed switch statements (converted
	to simple if-else) which run correctly on SGI64, but not LINUX, on this
	release, and possibly 6.0.  Also added better error handling for auxiliary 
	file input.
*/
#define MY_SEP_DOC 

#include <sep.main>
#include <stdio.h>
MAIN()
{
	int i1, n1, n1out;
	int i2, n2, n2out;
	int i3, n3, n3out,i4,n4,n5;
	int esize ;
	float rms, mean, scale, rmsout, sum;
	int oper;
	FILE* auxf;
	float *input;
	float *output;
	float *factors;

        sep_begin_prog();
	/* get input parameters */
	if(!hetch("n1","d",&n1)) seperr("n1");
	if(!hetch("n2","d",&n2)) seperr("n2");
	if(!hetch("n3","d",&n3)) seperr("n3");
	if(!hetch("n5","d",&n5)) n5=1;
	if(!hetch("n4","d",&n4)) n4=1;
	if(!hetch("esize","d",&esize)) esize = sizeof(float);
	rmsout = 2000.;
	getch("rms","f",&rmsout);
	oper = 0;
	getch("oper","d",&oper);

	/* allocate auxiliary arrays */ 
	input = (float *) alloc( n1 * sizeof(float));
	output = (float *) alloc( n1 * sizeof(float));
	factors = (float *) alloc( n2 * n3 * sizeof(float));

	/* set standard SEP paramters according to operation type */
	if( oper==1 ) {                          /* n1 zapped */
		n1out = n2;  n2out = n3;  n3out = 1;
	} else if( oper==2 ) {                   /* same as input */
		n1out = n1;  n2out = n2;  n3out = n3;
		if( auxin("bal") != NULL ) {           /* bal. factors from auxfile */
			if( n2*n3*esize != sreed( "bal", factors, n2*n3*esize) ) {
				seperr("FATAL: error reading from aux file bal\n");
			}
		} else {
			seperr("FATAL: must input aux file bal for oper=2\n");
		}
	} else {
		n1out = n1;  n2out = n2;  n3out = n3;  /* default */
	}

	/* output SEP standard parameters */ 
	putch("n1","d",&n1out);
	putch("n2","d",&n2out);
	putch("n3","d",&n3out);

	hclose();

	/* do the trace balancing according to the required option */ 
	/* loops over planes and over traces. Processes one trace at a time */
	for (i4=0; i4<n4*n5; i4++) {
	for (i3=0; i3<n3; i3++) {
		for (i2=0; i2<n2; i2++) {
			sreed( "in", input, n1 * 4);	/* reads one trace */

			/* compute scaling factor */
			if( oper==2 ) {
				scale = factors[i2+i3*n2];
			} else {
				sum = 0.;
				for (i1=0; i1<n1; i1++) {
					sum += input[i1]*input[i1];
				}
				mean = sum / n1;
				rms = sqrt(mean);
				if (rms != 0.) { 
					if (rmsout == 0.) rmsout = rms;
					scale = rmsout / rms;
				} else {
					scale = 0.;
				}
			}
 
			/* apply scaling factor */
			if( oper==1 ) {
				output[0] = scale;
				srite ("out",output,esize);
			} else {
				for (i1=0; i1<n1; i1++) {
					output[i1] = input[i1]*scale;
				}
				srite ("out",output,n1*esize);	/* output one trace */
			}
		}
    }
}
  sep_end_prog();
  return EXIT_SUCCESS;
}

