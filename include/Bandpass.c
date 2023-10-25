/*$

=head1 NAME

Bandpass,Lpfilt  - Bandpass dataset

=head1 SYNOPSIS

       Bandpass <in.H  flo=0. fhi=0.5/dt
                 nplo=6 nphi=6 phase=0 > out.H

       Lpfilt <in.H  f3db= order=6  > out.H

=head1 INPUT PARAMETERS

=over 4

=item    flo,fhi,f3db - real      

         low and high cutoff frequencies;
         flo for lowcut filter (no default); fhi=[0.5/dt] for highcut;
	     gain at flo and fhi is: -6db for zero-phase filter
				-3db for minimum-phase filter. flo and f3db are equivalent

=item   nplo,nphi,order - integer 

        number of poles for low and high cutoffs;
	    filter roll-off rate is proportional to number of poles 
      (order,nplo are equvilant)

=item    phase - integer     

         [0]: for zero-phase; =1 for minimum-phase

=back

=head1 DESCRIPTION

Butterworth bandpass filter.

=head1 EXAMPLED

       Bandpass <in.H in=stdin out=stdout flo=0. fhi=0.5/dt
                                    nplo=6 nphi=6 phase=0 > out.H

=head1 REFERENCE 

Technical reference: Oppenheim, A.V., and Schafer, R.W., 1975,
Digital signal processing, Prentice-Hall Inc.

=head1 SEE ALSO

L<Lpfilt>

=head1 CATEGORY

B<seis/filter>



=cut

  >*/

/*

  KEYWORDS	bandpass filter butterworth high-pass low-pass

  SEE ALSO
	 Lpfilt


% end of self documentation
  Author - Dave Hale
  EDIT HISTORY
       9-30-85	    stew    Convex version
       4-30-91	    steve   Saw version with no AP calls
       8-01-91	    lin	    Fix the bug and change the wrong comments
      14-09-99      james   Back? into C
                             fixed various bugs that had slipped in
			     stripped out highcut/lowcut subroutines

  Technical reference: Oppenheim, A.V., and Schafer, R.W., 1975,
	 Digital signal processing, Prentice-Hall Inc.

*/

#include <stdlib.h>
#include <stdio.h>
#include <seplib.h>

int main (argc, argv, envp) 
     int argc;
     char  **argv, **envp;

{
	int n1,n2,n3,n4,n5,n6,n7, esize, nplo, nphi, phase,n1pad,n,i1;
	int itr;
	float flo, fhi,d1;
	float *data, *newdata, *tempdata;

	/* prototypes for functions used internally */
	extern void lowcut( float, int, int, int, float*, float*, float*);
	extern void highcut( float, int, int, int, float*, float*, float*);

	/* initialize SEP stuff to handle command-line arguments and documentation */
	initpar (argc, argv); 
sep_add_doc_line("NAME");
sep_add_doc_line("    Bandpass,Lpfilt - Bandpass dataset");
sep_add_doc_line("");
sep_add_doc_line("SYNOPSIS");
sep_add_doc_line("           Bandpass <in.H  flo=0. fhi=0.5/dt");
sep_add_doc_line("                     nplo=6 nphi=6 phase=0 > out.H");
sep_add_doc_line("");
sep_add_doc_line("           Lpfilt <in.H  f3db= order=6  > out.H");
sep_add_doc_line("");
sep_add_doc_line("INPUT PARAMETERS");
sep_add_doc_line("    flo,fhi,f3db - real");
sep_add_doc_line("                 low and high cutoff frequencies;");
sep_add_doc_line("                 flo for lowcut filter (no default); fhi=[0.5/dt] for highcut;");
sep_add_doc_line("                     gain at flo and fhi is: -6db for zero-phase filter");
sep_add_doc_line("                                        -3db for minimum-phase filter. flo and f3db are equivalent");
sep_add_doc_line("");
sep_add_doc_line("    nplo,nphi,order - integer");
sep_add_doc_line("                number of poles for low and high cutoffs;");
sep_add_doc_line("                    filter roll-off rate is proportional to number of poles ");
sep_add_doc_line("              (order,nplo are equvilant)");
sep_add_doc_line("");
sep_add_doc_line("    phase - integer");
sep_add_doc_line("                 [0]: for zero-phase; =1 for minimum-phase");
sep_add_doc_line("");
sep_add_doc_line("DESCRIPTION");
sep_add_doc_line("    Butterworth bandpass filter.");
sep_add_doc_line("");
sep_add_doc_line("EXAMPLED");
sep_add_doc_line("           Bandpass <in.H in=stdin out=stdout flo=0. fhi=0.5/dt");
sep_add_doc_line("                                        nplo=6 nphi=6 phase=0 > out.H");
sep_add_doc_line("");
sep_add_doc_line("REFERENCE");
sep_add_doc_line("    Technical reference: Oppenheim, A.V., and Schafer, R.W., 1975, Digital");
sep_add_doc_line("    signal processing, Prentice-Hall Inc.");
sep_add_doc_line("");
sep_add_doc_line("SEE ALSO");
sep_add_doc_line("    Lpfilt");
sep_add_doc_line("");
sep_add_doc_line("CATEGORY");
sep_add_doc_line("    seis/filter");
sep_add_doc_line("");

        sep_begin_prog();
	doc(SOURCE);

	/* get input parameters */
	if (fetch("n1","d",&n1)==0) seperr("n1 required\n");
	if (fetch("n2","d",&n2)==0) n2=1;
	if (fetch("n3","d",&n3)==0) n3=1;
	if (fetch("n4","d",&n4)==0) n4=1;
	if (fetch("n5","d",&n5)==0) n5=1;
	if (fetch("n6","d",&n6)==0) n6=1;
	if (fetch("n7","d",&n7)==0) n7=1;
	if (fetch("esize","d",&esize)==0) esize=4;
	if (fetch("d1","f",&d1)==0) seperr("d1 required\n");
	if (getch("flo","f",&flo)==0)
	 if (getch("f3db","f",&flo)==0) flo=0.;
	if (getch("fhi","f",&fhi)==0) fhi=0.5/d1;
	if (getch("nplo","d",&nplo)==0) 
	  if (getch("order","d",&nplo)==0) nplo=6;
	if (getch("nphi","d",&nphi)==0) nphi=6;
	if (getch("phase","d",&phase)==0) phase=0;

	n=n2*n3*n4*n5*n6*n7;
 
	if (flo >= fhi) seperr("specify flo < fhi");
	if (nplo < 1) nplo = 1;
	if (nphi < 1) nphi = 1;
	putch("flo","f",&flo);
	putch("fhi","f",&fhi);

	flo = flo * d1;
	fhi = fhi * d1;

	if (flo < 0.0001 && fhi > 0.4999) seperr("must specify either flo or fhi");
	n1pad = n1+2;

	/* output header info */
	putch("nplo","d",&nplo);
	putch("nphi","d",&nphi);
	putch("phase","d",&phase);
	hclose();

	/* allocate pointer arrays */
	data  =malloc(n1pad*sizeof(float));
	newdata =malloc(n1pad*sizeof(float));
	tempdata=malloc(n1pad*sizeof(float));

	/* loop over traces */
	for (itr=0;itr<n;itr++)  {
		for (i1=0;i1<n1pad;i1++)  {    /* zero arrays */
			data[i1] = 0.; 
			newdata[i1] = 0.; 
			tempdata[i1] = 0.;
		}

		/* read in data */
		if (sreed("in",(data+2),n1*4) != n1*4) seperr("Problem reading data\n");

		/* bandpass data */
		if (flo > 0.000001)
			(void) lowcut(flo,nplo,phase,n1pad,data,newdata,tempdata);

		if (fhi < 4.999999)
			(void) highcut(fhi,nphi,phase,n1pad,data,newdata,tempdata);

		/* write out */
		if (srite("out",(data+2),n1*4) != n1*4) seperr("Problem writing data\n");
	}
       sep_end_prog();
	return 0;
}

/*****************************************************************************
*                           Subroutine highcut                               *
******************************************************************************
*                                                                            *
* highcut                                                                    *
*                                                                            *
*  DESCRIPTION                                                               *
*  Butterworth highcut (lowpass) filter                                      *;
*                                                                            
*  USAGE                                                                     *
*  void highcut(float fhi,int nphi,int phase,float *data,float *newdata,     *
*		float *tempdata, int n1)                                             *
******************************************************************************  
*  INPUT PARAMETERS                                                          *
*  float fhi           cutoff frequency;                                     *
*  int   nphi          number of poles;                                      *
*  int   phase         0=zero phase  1=min phase                             *
*  int   n1            length of data to be filtered                         *
*  float data[n1]      data to be filtered                                   *
*  float newdata[n1]   workspace                                             *
*  float tempdata[n1]  more workspace                                        *
******************************************************************************  
*  KEYWORDS	bandpass filter butterworth high-pass low-pass                   *
*                                                                            *
*  SEE ALSO                                                                  *
*	 Lpfilt Bandpass                                                         *
*                                                                            *
* end of self documentation                                                  *
******************************************************************************
*  Author - Dave Hale                                                        *
*  EDIT HISTORY                                                              *
*       9-30-85	    stew    Convex version                                   *
*       4-30-91	    steve   Saw version with no AP calls                     *
*       8-01-91	    lin	    Fix the bug and change the wrong comments        *
*      14-09-99     james   Back? into C                                     *
*                           fixed various bugs that had slipped in           *
*			                stripped out highcut/lowcut subroutines          *
*                                                                            *
*  Technical reference: Oppenheim, A.V., and Schafer, R.W., 1975,            *
*	 Digital signal processing, Prentice-Hall Inc.                           *
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define B(i,j) b[(j)*nb+(i)]
#define D(i,j) d[(j)*nd+(i)]
#define MOD(i,j) (i-j*(i/j))

/* static double pi=3.1415926535897932384626433832795028841971693; */

float *b,*d;
int ifhi=0,iflo=0;
int nb,nd,phase;

void highcut(float fhi,int nphi,int phase,int n1,float *data, float *newdata, 
	float *tempdata)
{
	int nodd,nb,j;
	float *b;
	float a,aa,aap4,dtheta,theta0;
	double c;

	int ihi,i1; 

	nodd = MOD(nphi,2);
	if (phase == 0)	{
		if (nodd != 0) nphi = (nphi+1)/2;
		else nphi = nphi/2;
		nodd = MOD(nphi,2); 
	}
	nb = (nphi+1)/2; 
	b=(float*) malloc(nb*5*sizeof(float));

	a = 2.0*tan(pi*fhi);  /* radius of poles in s-plane */ 
	aa = a*a;
	aap4 = aa+4.0;
	dtheta = pi/nphi;             /*  angular separation of poles  */ 
	if (nodd != 0) theta0 = 0.0;  /*  pole closest to real s axis */
	else theta0 = dtheta/2.0; 
	if (nodd != 0) {
		B(0,0) = a/(a+2.0);
		B(0,1) = B(0,0);
		B(0,2) = 0.0;
		B(0,3) = (a-2.0)/(a+2.0);
		B(0,4) = 0.0;
	}
	for (j=nodd;j<nb;j++) {
		c = 4.0*a*cos(theta0+j*dtheta);
		B(j,0) = aa/(aap4+c);
		B(j,1) = 2.0*B(j,0);
		B(j,2) = B(j,0);
		B(j,3) = (2.0*aa-8.0)/(aap4+c);
		B(j,4) = (aap4-c)/(aap4+c);
	}

	for (ihi=0;ihi<nb;ihi++) {
		for (i1=2;i1<n1;i1++)
			newdata[i1] = B(ihi,0)*data[i1]+B(ihi,1)*data[i1-1]+B(ihi,2)*
			data[i1-2]-B(ihi,3)*newdata[i1-1]-B(ihi,4)*newdata[i1-2];
		for (i1=0;i1<n1;i1++) data[i1] = newdata[i1];
	}

	if (phase == 0) {   /* highcut again in reverse */
		for (i1=2;i1<n1;i1++)
			tempdata[i1] = data[n1+1-i1];
		for (ihi=0;ihi<nb;ihi++) {
			for (i1=2;i1<n1;i1++)
				newdata[i1] = B(ihi,0)*tempdata[i1] + 
	              B(ihi,1)*tempdata[i1-1] + B(ihi,2)*tempdata[i1-2] - 
	              B(ihi,3)* newdata[i1-1] - B(ihi,4)* newdata[i1-2];
			for (i1=0;i1<n1;i1++) tempdata[i1] = newdata[i1];
		}
		for (i1=2;i1<n1;i1++) data[i1] = tempdata[n1+1-i1];
	}
}

/*****************************************************************************
*                           Subroutine lowcut                                *
******************************************************************************
*                                                                            *
* lowcut                                                                     *
*                                                                            *
*  DESCRIPTION                                                               *
*  Butterworth lowcut (highpass) filter                                      *
*                                                                            *
*  USAGE                                                                     *
*  void lowcut(float fhi,int nphi,int phase,int n1,float *data,              *
*       float *newdata, float *tempdata)                                     *
******************************************************************************  
*  INPUT PARAMETERS                                                          *
*  float flo           cutoff frequency;                                     *
*  int   nplo          number of poles;                                      *
*  int   phase         0=zero phase  1=min phase                             *
*  int   n1            length of data to be filtered                         *
*  float data[n1]      data to be filtered                                   *
*  float newdata[n1]   workspace                                             *
*  float tempdata[n1]  more workspace                                        *
******************************************************************************  
*  KEYWORDS	bandpass filter butterworth high-pass low-pass                   *
*                                                                            *
*  SEE ALSO                                                                  *
*	 Lpfilt Bandpass                                                         *
*                                                                            *
* end of self documentation                                                  *
******************************************************************************
*  Author - Dave Hale                                                        *
*  EDIT HISTORY                                                              *
*       9-30-85	    stew    Convex version                                   *
*       4-30-91	    steve   Saw version with no AP calls                     *
*       8-01-91	    lin	    Fix the bug and change the wrong comments        *
*      14-09-99     james   Back? into C                                     *
*                           fixed various bugs that had slipped in           *
*			                stripped out highcut/lowcut subroutines          *
*                                                                            *
*  Technical reference: Oppenheim, A.V., and Schafer, R.W., 1975,            *
*	 Digital signal processing, Prentice-Hall Inc.                           *
*****************************************************************************/

void lowcut(float flo,int nplo,int phase,int n1,float *data,float *newdata, 
	float *tempdata)
{
	int nodd,nd,j;
	float *d;
	float a,aa,aap4,dtheta,theta0,b1,b2,b3,den,e,ee,fno2;
	double c;
	int ilo,i1; 

	nodd = MOD(nplo,2);

	/* compute lowcut filter coefficients if required by transforming a 
	highcut filter with cutoff at half Nyquist */
	if (phase == 0) {
		if (nodd != 0) nplo = (nplo+1)/2;
		else nplo = nplo/2;
		nodd = MOD(nplo,2);
	}
	nd = (nplo+1)/2;
	d=(float*) malloc(nd*5*sizeof(float));
 
	fno2 = 0.25;   /*   Nyquist frequency over two?? */
	a = 2.*sin(pi*fno2)/cos(pi*fno2); aa = a*a; aap4 = aa+4;
	e = -cos(pi*(flo+fno2))/cos(pi*(flo-fno2)); ee = e*e;
	dtheta = pi/nplo;		        /*  angular separation of poles  */
	if (nodd != 0) theta0 = 0;		/*  pole closest to real s axis */
	else theta0 = dtheta/2;
	if (nodd != 0) {
		b1 = a/(a+2);
		b2 = (a-2)/(a+2);
		den = 1.0-b2*e;
		D(0,0) = b1*(1.-e)/den;
		D(0,1) = -D(0,0);
		D(0,2) = 0.;
		D(0,3) = (e-b2)/den;
		D(0,4) = 0.;
	} 
	for (j=nodd;j<nd;j++) {
		c = 4.*a*cos(theta0+j*dtheta);
		b1 = aa/(aap4+c);
		b2 = (2.*aa-8.)/(aap4+c);
		b3 = (aap4-c)/(aap4+c);
		den = 1.-b2*e+b3*ee;
		D(j,0) = b1*(1.-e)*(1.-e)/den;
		D(j,1) = -2.*D(j,0);
		D(j,2) = D(j,0);
		D(j,3) = (2.*e*(1.+b3)-b2*(1.+ee))/den;
		D(j,4) = (ee-b2*e+b3)/den;
	}

	/* lowcut filter */
	for (ilo=0;ilo<nd;ilo++) {
		for (i1=2;i1<n1;i1++)
			newdata[i1] = D(ilo,0)*data[i1]+D(ilo,1)* data[i1-1]+D(ilo,2)*
				data[i1-2]-D(ilo,3)*newdata[i1-1]- D(ilo,4)*newdata[i1-2];
		for (i1=0;i1<n1;i1++) data[i1] = newdata[i1];
	}
    
	/* lowcut again in reverse */
	if (phase == 0) {
		for (i1=2;i1<n1;i1++)
			tempdata[i1] = data[n1+1-i1];
		for (ilo=0;ilo<nd;ilo++) {
			for (i1=2;i1<n1;i1++)
				newdata[i1]=D(ilo,0)*tempdata[i1]+D(ilo,1)*tempdata[i1-1]+
					D(ilo,2)*tempdata[i1-2]-D(ilo,3)*newdata[i1-1]-
					D(ilo,4)*newdata[i1-2];
			for (i1=0;i1<n1;i1++) tempdata[i1]=newdata[i1];
		}
		for (i1=2;i1<n1;i1++)
			data[i1] = tempdata[n1+1-i1];
	}
	return;
}
