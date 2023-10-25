/*$

=head1 NAME

  Filter  - filter a dataset
 
=head1 SYNOPSIS

Filter <in.H filter=filter.H pad=zero > out.H
 
=head1 INPUT PARAMETERS

=over 4

=item  n1,n2,n3 - integer   

       cube dimensions (n1<4096)

=item  n1       - integer   

       length of the filter(from auxilary file)

=item  pad      - string    

       zero: pad data with zeros 
       duplicate: pad with samples at ends of data

=item  filter       - char*   

       name of the auxiliary history file containing the filter coefficients

=back 

=head1 DESCRIPTION

  Filtering over 1 dimension performed in the frequency domain
	The filter is input as the history file

=head1 COMMENTS
        The length of the data when performing the filtering is zero padded 
        to be the next power of 2 greater than or equal to 
        max(5/4*n1,n1wavelet).  This avoids wraparound artifacts.

=head1 CATEGORY

B<seis/filter>

=cut

>*/

/*
 %

  Author - Peter Mora
 
  ----------
   Keyword:  filter synthetic convolve wavelet : Wavelet
  ----------
  1/8/92	jon	junk nt,nx anachronisms
  Revised:	Hector Urdaneta  (SEP) 8/26/94 
 		Commented out getch2 calls and changed them for auxpar
 		in order to be compatible with SeplibV2. 
  Revised:      stew (Mobil/SEP) 8/28/97
                Added option to control padding; Changed to saw
  Revised:      james 9/18/1999
                Rewrote in C, now applies shift implied by otfilt
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <seplib.h>
#include <sepmath.h>
#include <sepfft.h>

/* prototype for subroutine used internally */
int pad2(int);

int main (argc, argv, envp) 
	int argc;
	char  **argv, **envp;

{
	int ntfilt;
	float dtfilt,otfilt;
	int n,nt,n2,n3,n4,n5,n6,ntotal;
	float dt;
	char pad[20];
	float *data,*filt;
	complex *cdata,*cfilt;
	int it,i,j,ier;
	double dw,w;

	/* call SEP routines for initialization and documentation */
	initpar (argc, argv); 
        sep_begin_prog();
sep_add_doc_line("NAME");
sep_add_doc_line("      Filter  - filter a dataset");
sep_add_doc_line("");
sep_add_doc_line("SYNOPSIS");
sep_add_doc_line("    Filter <in.H filter=filter.H pad=zero > out.H");
sep_add_doc_line("");
sep_add_doc_line("INPUT PARAMETERS");
sep_add_doc_line("    n1,n2,n3 - integer");
sep_add_doc_line("               cube dimensions (n1<4096)");
sep_add_doc_line("");
sep_add_doc_line("    n1 - integer");
sep_add_doc_line("               length of the filter(from auxilary file)");
sep_add_doc_line("");
sep_add_doc_line("    pad - string");
sep_add_doc_line("               zero: pad data with zeros ");
sep_add_doc_line("               duplicate: pad with samples at ends of data");
sep_add_doc_line("");
sep_add_doc_line("    filter - char*");
sep_add_doc_line("               name of the auxiliary history file containing the filter coefficients");
sep_add_doc_line("");
sep_add_doc_line("DESCRIPTION");
sep_add_doc_line("      Filtering over 1 dimension performed in the frequency domain");
sep_add_doc_line("            The filter is input as the history file");
sep_add_doc_line("");
sep_add_doc_line("COMMENTS");
sep_add_doc_line("            The length of the data when performing the filtering is zero padded ");
sep_add_doc_line("            to be the next power of 2 greater than or equal to ");
sep_add_doc_line("            max(5/4*n1,n1wavelet).  This avoids wraparound artifacts.");
sep_add_doc_line("");
sep_add_doc_line("CATEGORY");
sep_add_doc_line("    seis/filter");
sep_add_doc_line("");

	doc(SOURCE);

	/* get parameters and set defaults */
	if (fetch("n1","d",&nt)==0) seperr("Need data n1\n");
	if (fetch("d1","f",&dt)==0) seperr("Need data d1\n");
	if (fetch("n2","d",&n2)==0) n2=1;
	if (fetch("n3","d",&n3)==0) n3=1;
	if (fetch("n4","d",&n4)==0) n4=1;
	if (fetch("n5","d",&n5)==0) n5=1;
	if (fetch("n6","d",&n6)==0) n6=1;
	ntotal=n2*n3*n4*n5*n6;
 
	/* get prameters from auxiliary file (for filter coefficients) */  
	if (auxpar("n1","d",&ntfilt,"filter")==0) seperr("Need filter n1\n");
	if (auxpar("o1","f",&otfilt,"filter")==0) otfilt=0.0;
	if (auxpar("d1","f",&dtfilt,"filter")==0) dtfilt=dt;

	/* check sampling rate of data and filter */
	if (fabs(dtfilt-dt) < 0.000001) {
		putlin("WARNING: data and filter have different d1's, using data's");
		dtfilt=dt;
	}

	/* get parameter for pad option */
	if (getch("pad","s",pad)==0) strcpy(pad,"zero");
	if (strcmp(pad,"zero")!=0 && strcmp(pad,"duplicate")!=0) { 
		putlin("WARNING: don't understand pad, using pad=zero");
		strcpy(pad,"zero");
	}
	if(ntfilt!=nt) putlin("REMARK: data and filter are different lengths");
 
	/* do the padding */ 
	n=2*MAX(pad2(nt*5/4),pad2(ntfilt));
	if(ntfilt!=n) putlin("REMARK: filter length will be padded for filtering");
	if(nt!=n)     putlin("REMARK: data length will be padded for filtering");

	/* write number of samples in output data */
	putch("nt_after_padding_for_filtering","d",&n);
	hclose();
 
	/* allocate working space */ 
	data=(float *) malloc(n*sizeof(float));
	filt=(float *) malloc(n*sizeof(float));
	cdata=(complex *) malloc(n*sizeof(complex));
	cfilt=(complex *) malloc(n*sizeof(complex));

	/*  read wavelet */
	filt[0]=1. ; 
	for (it=1; it<n; it++)  filt[it]=0.;
	ier= sreed("filter", filt, ntfilt*4 );
	for (it=0; it<n; it++)  cfilt[it]=scmplx(filt[it],0.);

	/*  fft wavelet */
	cefft(cfilt,n,1.,1./sqrt(1.*n));

	/* additional time delay */
	if(otfilt!=0.) {
		dw=2*pi/(n*(double) dtfilt); w=0.;
		for (it=0; it<n; it++) { 
			cfilt[it]=scmult( cfilt[it], sciexp(w*otfilt)); 
			if(it==n/2) w=(-w); 
			w=w+dw;
		}
	}

	/* loop over traces */
	for (i=0;i<ntotal;i++) {
		ier= sreed("in",data,4*nt);
		if (strcmp(pad,"zero")==0) 
			for (j=nt;j<n;j++) data[j] = 0.;
		else {
			for (j=nt;j<(nt+n)/2;j++) data[j]=data[nt];
			for (j=(nt+n)/2;j<n; j++) data[j]=data[1];
		} 

		/* make complex data and compute fft */
		for (it=0; it<n; it++)  cdata[it]=scmplx(data[it],0.);
		cefft(cdata,n,1.,sqrt(1./sqrt(1.*n)));

		/* do filtering and inverse fft */
		for (it=0; it<n; it++)  cdata[it]=scmult(cdata[it],cfilt[it]);
		cefft(cdata,n,-1.,sqrt(1./sqrt(1.*n)));

		/* output fitlered trace */
		for (it=0; it<n; it++)  data[it]=cdata[it].re;
		ier=srite("out",data,4*nt);
	}
        sep_end_prog();
	return 0;
}

int pad2(int n)
{
	int i;

	i=1;
	while (i<n) {
		i=2*i;
	}
	return i;
}
