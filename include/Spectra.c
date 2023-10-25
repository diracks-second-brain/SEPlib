/*$

=head1 NAME

Spectra -  Calculate spectrum

=head1 SYNOPSIS

Spectra < in.h [in= out= j1= j2= j3= d1= phase=] > out.H

=head1 INPUT PARAMETERS

=over 4

=item  n1 - integer   

       on output (n1=(nearest power of two)/(2*j1)+1)

=item  n2,n3   - integer   

       like Window

=item  esize   - integer   

       4=default=real input; 8=optional complex input

=item  j1,j2,j3 -integer   

        like Window but spectra will be added, not subsampled,
        effectively smoothing the spectra

=item  d1       -real      

       on output will be replaced by j1/2*dt

=item  phase    -file      

       output auxiliary file with phase spectra

=item  mode     -char      

       mode=all defaults j2=n2 & j3=n3 to reproduce 
       functionality of old Spectrum program (not the default)

=back

=head1 DESCRIPTION

Obtain averaged amplitude spectra.
Spectrum on 1=axis, smoothing on all axes.

=head1 SEE ALSO

L<Ftplot>

=head1 CATEGORY

B<seis/filter>

=cut
  >*/


/*
  KEYWORDS        average amplitude spectrum phase

  WHERE
       ./cube/seis/Spectra.rst

  Author - Peter Mora
  EDIT HISTORY
      8-25-85	     jon   putch o1=0. title="amplitude spectrum"
      9-22-87       clem   maxnt to 8192, as in cfft1
      1-26-88       john   handle complex input data
      8-10-89     carlos   saw, j3
      6-26-91     carlos   phase spectrum
      8-1-99     Bob       Switched to f90
      8-5-99     Bob       Switch to cfft (if it doesn't work right change
                           sign, untested)
      9-14-99      james   Rewritten in C
                           Most original functionality intact
			   Remerged Spectra and Spectrum with mode=all
			   Changed way phase spectra are calculated
			   Phase spectra now acknowledges o1

 ----------
  Keyword:  average amplitude spectrum phase
 ----------
*/

#include <stdlib.h>
#include <stdio.h>
#include <seplib.h>
#include <sepmath.h>
#include <sepfft.h>

int main (argc, argv, envp) 
     int argc;
     char  **argv, **envp;
{
	int j1,j2,j3, n1,n2,n3, esize;
	int n1pad,n1out,n2out,n3out,eout;
	float o1,d1,d2,d3,d1out,o1out,d2out,d3out;
	int i2,i3,i,i2out,i3out,ier,iphase;
	complex cdelay;
	char *name;
	char mode[20],phaze[20];
	float *temp,*spec;
	complex *cdata,*phase;
	int *inorm;

	/* SEP stuff to handle command line arguments and documentation */
	initpar (argc, argv); 
sep_add_doc_line("NAME");
sep_add_doc_line("    Spectra - Calculate spectrum");
sep_add_doc_line("");
sep_add_doc_line("SYNOPSIS");
sep_add_doc_line("    Spectra < in.h [in= out= j1= j2= j3= d1= phase=] > out.H");
sep_add_doc_line("");
sep_add_doc_line("INPUT PARAMETERS");
sep_add_doc_line("    n1 - integer");
sep_add_doc_line("               on output (n1=(nearest power of two)/(2*j1)+1)");
sep_add_doc_line("");
sep_add_doc_line("    n2,n3 - integer");
sep_add_doc_line("               like Window");
sep_add_doc_line("");
sep_add_doc_line("    esize - integer");
sep_add_doc_line("               4=default=real input; 8=optional complex input");
sep_add_doc_line("");
sep_add_doc_line("    j1,j2,j3 -integer");
sep_add_doc_line("                like Window but spectra will be added, not subsampled,");
sep_add_doc_line("                effectively smoothing the spectra");
sep_add_doc_line("");
sep_add_doc_line("    d1 -real");
sep_add_doc_line("               on output will be replaced by j1/2*dt");
sep_add_doc_line("");
sep_add_doc_line("    phase -file");
sep_add_doc_line("               output auxiliary file with phase spectra");
sep_add_doc_line("");
sep_add_doc_line("    mode -char");
sep_add_doc_line("               mode=all defaults j2=n2 & j3=n3 to reproduce ");
sep_add_doc_line("               functionality of old Spectrum program (not the default)");
sep_add_doc_line("");
sep_add_doc_line("DESCRIPTION");
sep_add_doc_line("    Obtain averaged amplitude spectra. Spectrum on 1=axis, smoothing on all");
sep_add_doc_line("    axes.");
sep_add_doc_line("");
sep_add_doc_line("SEE ALSO");
sep_add_doc_line("    Ftplot");
sep_add_doc_line("");
sep_add_doc_line("CATEGORY");
sep_add_doc_line("    seis/filter");
sep_add_doc_line("");

	doc(SOURCE);
        sep_begin_prog();

	/* get parameters */
	if (fetch("n1","d",&n1)==0) n1=1;
	if (fetch("n2","d",&n2)==0) n2=1;
	if (fetch("n3","d",&n3)==0) n3=1;
	if (fetch("esize","d",&esize)==0) esize=4;
  

	if (NULL == (name = strrchr(argv[0], '/'))) name=argv[0];
	if(0==strcmp(name,"Spectrum")) strcpy(mode,"all");

	ier=getch("mode","s",&mode);
	if(strcmp(mode,"all")==0) { j1=1; j2=n2; j3=n3; }
	else{ j1=1; j2=1;  j3=1;  }

	ier=getch("j1","d",&j1)==0;
	ier=getch("j2","d",&j2)==0;
	ier=getch("j3","d",&j3)==0;
	iphase=0;  if (getch("phase","s",phaze) != 0) iphase=1;

	if (fetch("d1","f",&d1)==0) d1=0.004;
	if (fetch("d2","f",&d2)==0) d2=1.;
	if (fetch("d3","f",&d3)==0) d3=1.;
	if (fetch("o1","f",&o1)==0) o1=0.;

	/*  calculate n1out,n2out,n3out etc...  */
	n1pad=1; o1out=0.0; eout=4;
	while (n1pad < n1) {
		n1pad *= 2;
		if(n1!=n1pad) putlin("WARNING: data padded to a power of 2");
	}
	d1out=j1/(n1pad*d1);
	d2out=d2*j2;
	d3out=d3*j3;
	n1out=n1pad/(2*j1)+1;
	n2out=(n2-1)/j2+1;
	n3out=(n3-1)/j3+1;

	/* allocate working space */
	cdata  = (complex*) malloc(n1pad*sizeof(complex));
	temp   = (float*) malloc(n1*sizeof(float));
	spec   = (float*) malloc(n1pad*n2out*sizeof(float));
	inorm  = (int*) malloc(n2out*sizeof(int));
	if (iphase) phase = (complex*) malloc(n1pad*n2out*sizeof(complex));

	/* put parameter values out */
	putch("n1","d",&n1out);
	putch("n2","d",&n2out);
	putch("n3","d",&n3out);
	putch("d1","f",&d1out);
	putch("d2","f",&d2out);
	putch("d3","f",&d3out);
	putch("o1","f",&o1out);
	putch("esize","d",&eout);

	if (iphase) {
		auxputch("n1","d",&n1out,"phase");
		auxputch("n2","d",&n2out,"phase");
		auxputch("n3","d",&n3out,"phase");
		auxputch("d1","f",&d1out,"phase");
		auxputch("d2","f",&d2out,"phase");
		auxputch("d3","f",&d3out,"phase");
		auxputch("o1","f",&o1out,"phase");
		auxputch("esize","d",&eout,"phase");
	}
	hclose();

	for (i=0;i<n1pad*n2out;i++) spec[i] =0.;
	if (iphase) for (i=0;i<n1pad*n2out;i++) phase[i] =scmplx(0.,0);
	for (i=0;i< n2out;i++) inorm[i] =0;
  
	/* loop over all traces */
	for (i3=0;i3<n3;i3++) { 
		i3out=i3/j3;
		for (i2=0;i2<n2;i2++) {
			i2out=i2/j2;
 
			/* read in data-trace */
			if(sreed("in",temp,n1*esize)!=n1*esize) 
				seperr("Problem reading data\n");
			for (i=0;i<n1pad;i++) cdata[i]=scmplx(0.,0.);
			for (i=0;i<n1;i++) {
				if (esize==4) cdata[i].re += temp[i];
				else cdata[i]=scadd(cdata[i],scmplx(temp[2*i],temp[2*i+1]));
			}
    
			/* calculate spectrum */
			cefft(cdata,n1pad,1,1./sqrt(1.*n1pad));

			/* add into spec */
			for (i=0;i<n1pad;i++) spec[i2out*n1pad+i/j1] += sccabs(cdata[i]);
			inorm[i2out] += 1;
			if (iphase) for (i=0;i<n1pad;i++) {
				cdelay=scexp(scmplx(0., (2*pi*i*d1out*o1)));
				cdata[i]=scmult(cdata[i],cdelay);
				phase[i2out*n1pad+i]= scadd(cdata[i],phase[i2out*n1pad+i]);
			}
		}

		if (((i3/j3) != ((i3+1)/j3)) || (i3+1==n3)) {
			for (i2=0;i2<n2out;i2++)    /*   normalize */
				for (i=0;i<n1pad;i++) spec[i+i2*n1pad] /= (float) inorm[i2];

			for (i=0;i<n2out;i++)       /*    write output */
				if (srite("out",(spec+i*n1pad),n1out*4) != n1out*4) 
					seperr("Problem writing data\n");

			if (iphase) {
				for (i=0;i<n1pad*n2out;i++) spec[i] =atan2(phase[i].im,phase[i].re);
				for (i=0;i<n2out;i++)       /*    write phase */
					if (srite("phase",(spec+i*n1pad),n1out*4) != n1out*4) 
						seperr("Problem writing phase\n");
			}

			/*  then  zero output */
			for (i=0;i<n1pad*n2out;i++) spec[i] =0.;
			for (i=0;i<n2out;i++) inorm[i]=0;
			if (iphase) for (i=0;i<n1pad*n2out;i++) phase[i] =scmplx(0.,0);
		}
	}
         sep_end_prog();
	return 0;
}
