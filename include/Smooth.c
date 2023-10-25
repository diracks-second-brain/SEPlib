
 /*$
=head1 NAME

 Smooth - 2-D smoothing
 
=head1 SYNOPSIS

        Smooth < in.h [options] > out.H
 
=head1  INPUT PARAMETERS

=over 4

   Halfwidth is the width in sample points of a rectangle which 
             when convolved with itself gives a triangle:


=item   rect1,rect2,rect3     - integer 

	[1]: halfwidth on 1- 2- and 3-axis respectively (=1 means no smoothing)

        NOTE: 3-axis smoothing is done by direct
         convolution, not by integration - so
         may be slower for large rect3. If in 
          doubt, Transp and Smooth with rect2.

=item tridiag  - integer 

	[1]: =1 to smooth with tridiagonal solver. (NOT 3-axis)

=item repeat- integer 

	[1]: number of times to repeat filtering, good for 
       Gaussian smoothing. (NOT 3-axis)

=item  sum3  - integer 

	[0]: =0  to repeat over n3 axis.
		 =1  to sum over n3 axis

=item   agc - integer 
  
	[0]: =1 to use smoothed data as divisor gain for data.

=item   absval    - integer 

	[0]: =1 means smooth absolute values of input.

=item   scaleup   - integer 

	[0]: =1 means scale up to compensate for bandwidth reduction.
         =0 to preserve scale of zero frequency component.

=item  Difference stencils applied after smoothing:

=item   diff1  - integer  

	[0]:  =1  apply 2-pt difference stencil on 1-axis:
        data(i1+1,i2)=data(i1+1,i2)-data(i1,i2)

=item   diff2- integer  

	[0]:  =1  apply 2-pt difference stencil on 2-axis:
        data(i1,i2+1)=data(i1,i2+1)-data(i1,i2)

=item   maxmem - integer  

  [100000000]: Maximum memory to allocate.  

=back

=head1  DESCRIPTION

 Smooth 2-D with triangle or 1/(Dxx - const), and/or SUM_n3, or AGC 
 
=head1  COMMENTS

       Dip enhancement: use options='rect1=100 tridiag=1 diff1=1 diff2=1'
       To reduce memory usage: Try smaller data/filter, repeat>1, tridiag=1.
 
=head1 CATEGORY

B<seis/filter>

=cut
  >*/

/*

  KEYWORDS   smooth triangle tridiagonal agc filter 
 
   AUTHOR
       jon 1/19/90, 8/11/92
 
   WHERE
       ./cube/seis/Smooth.rs
 
  david -- added "rect3" option 8/19/92
  david -- changed default to scaleup=0  9/15/92
  david -- added n2=1, n3=1 from "either" option  11/20/93
  david -- changed n2=1, n3=1 back to "history" option  7/95
  james -- rewrote in C  9/19/99
           added "repeat" option
           fixed scaleup bug
           fixed rect3 so it now behaves like rect1/rect2
  morgan - 8Nov2002 - Added maxmem check.
*/

#include <stdlib.h>
#include <stdio.h>
#include <seplib.h>

/* NOTE: C TYPE STORAGE  */
#define DATA(i,j) data[n1*(i)+(j)]
#define SMOO(i,j) smoo[n1*(i)+(j)]
#define SUM(i,j)  sum[n1*(i)+(j)]
#define TEMP(i,j,k) temp[n12*(reflect(b3,i))+n1*(j)+(k)]

#define TRIS_NMAX 8192

/* prototypes for functions used internally */
extern int tris(int,float,float,float,float,float,float*,float*);
float *e, *f, *deni;
extern void copy(int,float*,float*);
extern void triangle(int,int,int,float*,float*);
float *pp, *qq, *tt;
extern void leaky (float,int,int,float*,float*);
float *vecin,*vecout;
extern void boxconv(int,int,float*,float*);
float *bb;
extern int reflect(int,int);

int main (argc, argv, envp) 
     int argc;
     char  **argv, **envp;
{
	int n1,n2,n3,n4,n5,n6,ntotal,n12,n3out,nread;
	int rect1,rect2,rect3;
	int tridiag,scaleup,diff1,diff2,agc;
	int absval,sum3;
	int b3,maxrect;
	int j, i1,i2,i3, count,irepeat,nrepeat;
	int maxmem, memcount;
	float *data,*smoo,*sum,*temp;
	float dist1,dist2,scale;
	float scale3;

	/* SEP stuff to handle command line arguments and documentation */
	initpar (argc, argv); 
sep_add_doc_line("NAME");
sep_add_doc_line("     Smooth - 2-D smoothing");
sep_add_doc_line("");
sep_add_doc_line("SYNOPSIS");
sep_add_doc_line("            Smooth < in.h [options] > out.H");
sep_add_doc_line("");
sep_add_doc_line("INPUT PARAMETERS");
sep_add_doc_line("           Halfwidth is the width in sample points of a rectangle which ");
sep_add_doc_line("                     when convolved with itself gives a triangle:");
sep_add_doc_line("");
sep_add_doc_line("    rect1,rect2,rect3 - integer");
sep_add_doc_line("                [1]: halfwidth on 1- 2- and 3-axis respectively (=1 means no smoothing)");
sep_add_doc_line("");
sep_add_doc_line("                NOTE: 3-axis smoothing is done by direct");
sep_add_doc_line("                 convolution, not by integration - so");
sep_add_doc_line("                 may be slower for large rect3. If in ");
sep_add_doc_line("                  doubt, Transp and Smooth with rect2.");
sep_add_doc_line("");
sep_add_doc_line("    tridiag - integer");
sep_add_doc_line("                [1]: =1 to smooth with tridiagonal solver. (NOT 3-axis)");
sep_add_doc_line("");
sep_add_doc_line("    repeat- integer");
sep_add_doc_line("                [1]: number of times to repeat filtering, good for ");
sep_add_doc_line("               Gaussian smoothing. (NOT 3-axis)");
sep_add_doc_line("");
sep_add_doc_line("    sum3 - integer");
sep_add_doc_line("                [0]: =0  to repeat over n3 axis.");
sep_add_doc_line("                         =1  to sum over n3 axis");
sep_add_doc_line("");
sep_add_doc_line("    agc - integer");
sep_add_doc_line("                [0]: =1 to use smoothed data as divisor gain for data.");
sep_add_doc_line("");
sep_add_doc_line("    absval - integer");
sep_add_doc_line("                [0]: =1 means smooth absolute values of input.");
sep_add_doc_line("");
sep_add_doc_line("    scaleup - integer");
sep_add_doc_line("                [0]: =1 means scale up to compensate for bandwidth reduction.");
sep_add_doc_line("                 =0 to preserve scale of zero frequency component.");
sep_add_doc_line("");
sep_add_doc_line("    Difference stencils applied after smoothing:");
sep_add_doc_line("    diff1 - integer");
sep_add_doc_line("                [0]:  =1  apply 2-pt difference stencil on 1-axis:");
sep_add_doc_line("                data(i1+1,i2)=data(i1+1,i2)-data(i1,i2)");
sep_add_doc_line("");
sep_add_doc_line("    diff2- integer");
sep_add_doc_line("                [0]:  =1  apply 2-pt difference stencil on 2-axis:");
sep_add_doc_line("                data(i1,i2+1)=data(i1,i2+1)-data(i1,i2)");
sep_add_doc_line("");
sep_add_doc_line("    maxmem - integer");
sep_add_doc_line("          [100000000]: Maximum memory to allocate.  ");
sep_add_doc_line("");
sep_add_doc_line("DESCRIPTION");
sep_add_doc_line("     Smooth 2-D with triangle or 1/(Dxx - const), and/or SUM_n3, or AGC ");
sep_add_doc_line("");
sep_add_doc_line("COMMENTS");
sep_add_doc_line("           Dip enhancement: use options='rect1=100 tridiag=1 diff1=1 diff2=1'");
sep_add_doc_line("           To reduce memory usage: Try smaller data/filter, repeat>1, tridiag=1.");
sep_add_doc_line("");
sep_add_doc_line("CATEGORY");
sep_add_doc_line("    seis/filter");
sep_add_doc_line("");

        sep_begin_prog();
	doc(SOURCE);

	/* get parameters */
	if (hetch("n1","d",&n1)==0) seperr("Need data n1\n");
	if (hetch("n2","d",&n2)==0) n2=1;
	if (hetch("n3","d",&n3)==0) n3=1;
	if (hetch("n4","d",&n4)==0) n4=1;
	if (hetch("n5","d",&n5)==0) n5=1;
	if (hetch("n6","d",&n6)==0) n6=1;
	if (getch("rect1","d",&rect1)==0) rect1=1;
	if (getch("rect2","d",&rect2)==0) rect2=1;
	if (getch("rect3","d",&rect3)==0) rect3=1;
	if (getch("tridiag","d",&tridiag)==0) tridiag=1;
	if (getch("scaleup","d",&scaleup)==0) scaleup=0;
	if (getch("diff1","d",&diff1)==0) diff1=0;
	if (getch("diff2","d",&diff2)==0) diff2=0;
	if (getch("agc","d",&agc)==0) agc=0;
	if (getch("absval","d",&absval)==0) absval=agc;
	if (getch("sum3","d",&sum3)==0) sum3=0;
	if (getch("repeat","d",&nrepeat)==0) nrepeat=1;
	if (getch("maxmem","d",&maxmem)==0) maxmem=200000000;

	ntotal=n3*n4*n5*n6; n12=n1*n2;

	memcount = 3*n12*sizeof(float) + n12*sizeof(float)*(2*rect3-1);
	if( tridiag==0 ) {
		memcount += ( (n12+maxrect-1) + (n12+2*maxrect-2) + n12
                 + maxrect*(maxrect+n12) )*sizeof(float);
	} else {
		memcount += 5*n12*sizeof(float);
	}

	if( memcount > maxmem ) {
		fprintf(stderr,"ERROR: memcount>maxmem. Tried to allocate %d bytes\n",memcount);
		seperr("Try smaller data/filter, repeat>1, tridiag=1\n");
	}

	/* allocate working space */
	data=(float *) malloc(n12*sizeof(float));
	smoo=(float *) malloc(n12*sizeof(float));
	sum =(float *) malloc(n12*sizeof(float));
	temp=(float *) malloc(n12*sizeof(float)*(2*rect3-1));

	dist1 = 1. + .5 * rect1;
	dist2 = 1. + .6 * rect2;
	if (agc>0  && sum3>0) seperr("I cannot do both agc>0 and sum3>0");
	if (agc>0) putlin("AGC:  Automatic Gain Control");
	if (absval>0) putlin("Absolute values of data used.");
	if (tridiag==0) putlin("Data smoothed with triangle().");
	else putlin("Data smoothed with tridiagonal solver leaky().");

	/* output processing information */
	putch("Data smoothed with: rect1","i",&rect1);
	putch("Data smoothed with: rect2","i",&rect2);
	putch("Data smoothed with: rect3","i",&rect3);

	if( sum3 > 0 ) { 
		n3out=1; 
		putch("n3","i",&n3out);  
	}
	hclose();

	b3= 2*rect3-1;
	scale3= 1./((float) rect3);

	maxrect=MAX(MAX(rect1,rect2),rect3);

	/* allocate more working space */
	if (tridiag==0) {
		pp=(float*) malloc((n12+maxrect  -1)    *sizeof(float));
		qq=(float*) malloc((n12+2*maxrect-2)    *sizeof(float));
		tt=(float*) malloc((n12)                *sizeof(float));
		bb=(float*) malloc(maxrect*(maxrect+n12)*sizeof(float));
	} else {
		vecin =(float*) malloc(n12*sizeof(float));
		vecout=(float*) malloc(n12*sizeof(float));
		e     =(float*) malloc(n12*sizeof(float));
		f     =(float*) malloc(n12*sizeof(float));
		deni  =(float*) malloc(n12*sizeof(float));
	}

	/* initial temp load */
	nread=0;
	for( i3=0; i3<rect3; i3++ ) {
		if( nread < ntotal ) {    /* ntotal = # slices in data */
			count = sreed("in", data, 4*n12 );
			nread++;   /* Read in a slice if we're not to data end */
		} else {     /* Set data=temp if off end of data cube */
			for (i2=0;i2<n2;i2++)
				for (i1=0;i1<n1;i1++)
					DATA(i2,i1) = TEMP(n3,i2,i1);
		}
		for (i2=0;i2<n2;i2++) {
			for (i1=0;i1<n1;i1++) {
				TEMP(rect3-1+i3,i2,i1) = DATA(i2,i1);
				if( i3!=rect3-1) 
				  TEMP(rect3-2-i3,i2,i1)= DATA(i2,i1);
			}
		}
	} 

	for( i3=0; i3<ntotal; i3++) {
		if( i3!=0 ) {

			/* shift everything along */
			for (j=0;j<b3-1;j++)
				for (i2=0;i2<n2;i2++)
					for (i1=0;i1<n1;i1++)
						TEMP(j,i2,i1) = TEMP(j+1,i2,i1);

			/* load next plane */
			if (nread <ntotal) { 
				count = sreed( "in", data, 4*n12 ); nread++; 
			} else {
				for (i2=0;i2<n2;i2++)
					for (i1=0;i1<n1;i1++)
						DATA(i2,i1) = TEMP(nread,i2,i1);
			}
			for (i2=0;i2<n2;i2++)
				for (i1=0;i1<n1;i1++)
					TEMP(b3-1,i2,i1)=DATA(i2,i1);
		}

		/* smooth temp and add into data */
		for (i2=0;i2<n2;i2++)
			for (i1=0;i1<n1;i1++)
				DATA(i2,i1) = TEMP(rect3-1,i2,i1);

		for (j=0;j<rect3-1;j++)
			for (i2=0;i2<n2;i2++)
				for (i1=0;i1<n1;i1++)
					DATA(i2,i1) += ((float) (j+1))/((float) rect3)* 
						(TEMP(j,i2,i1)+TEMP(2*rect3-2-j,i2,i1));

		for (i2=0;i2<n2;i2++)
			for (i1=0;i1<n1;i1++) 
				DATA(i2,i1) *= scale3;

		for (i2=0;i2<n2;i2++)
			for (i1=0;i1<n1;i1++)
				if (absval > 0) SMOO(i2,i1) = abs( DATA(i2,i1));
				else            SMOO(i2,i1) =      DATA(i2,i1) ;

		for (irepeat=0;irepeat<nrepeat;irepeat++) {
			if (rect1>1  &&  n1>1) {
				for (i2=0;i2<n2;i2++)
					if (tridiag==0) triangle(rect1,1,n1,(smoo+i2*n1),(smoo+i2*n1));
					else            leaky(dist1,1, n1, (smoo+i2*n1), (smoo+i2*n1));
			}

			if (rect2>1  &&  n2>1) {
				for (i1=0;i1<n1;i1++)
					if (tridiag==0) triangle(rect2, n1,n2, (smoo+i1), (smoo+i1));
					else            leaky(   dist2, n1,n2, (smoo+i1), (smoo+i1));
			}

			if (scaleup>0) {
				scale =1.*sqrt(rect1*rect1 * rect2*rect2 * rect3*rect3);
				for (i2=0;i2<n2;i2++)
					for (i1=0;i1<n1;i1++)
						SMOO(i2,i1) *= scale;
			}
    
			if (diff1>0) {
				for (i2=0;i2<n2;i2++)
					for (i1=n1-1;i1>=1;i1--)
						SMOO(i2,i1) -= SMOO(i2,i1-1);
				for (i2=0;i2<n2;i2++)
					SMOO(i2,0) = 0.;
			}
    
			if (diff2>0) {
				for (i1=0;i1<n1;i1++) 
					for (i2=n2-1;i2>=1;i2--) 
						SMOO(i2,i1) -= SMOO(i2-1,i1); 
				for (i1=0;i1<n1;i1++) 
					SMOO(0 ,i1)  = 0.;
			}
		}

		if (sum3==0) {
			if (agc > 0) {
				for (i2=0;i2<n2;i2++)
					for (i1=0;i1<n1;i1++)
						if (SMOO(i2,i1) != 0)
							SMOO(i2,i1) = DATA(i2,i1) / SMOO(i2,i1);
			}
			count = srite( "out", smoo, 4*n1*n2 );
		} else {
      		for (i2=0;i2<n2;i2++)
				for (i1=0;i1<n1;i1++)
					SUM(i2,i1) += SMOO(i2,i1);
		}
	}
	if (sum3 > 0)
	count = srite( "out", sum, 4*n1*n2 );

        sep_end_prog();
	return 0;
}

int reflect(int n,int i)
{
  int j;
  j=i;
  if (n==1) return 0;
  while (j>n-1  ||  j<0) {
    if  (j>n-1) j= 2*n-1-j;
    if  (j<0)   j=  (-j); 
  } 
  return j;
}

/*************************************************************************
*                          Subroutine triangle                           *
**************************************************************************
*                        Convolve with triangle                          *
**************************************************************************
*                                                                        *
*  input:  nr    rectangle width (points) (Triangle base twice as wide.) *
*  input:  uu(m1,i2),i2=1,n12      is a vector of data.                  *
*  output: vv(m1,i2),i2=1,n12      may be on top of uu                   * 
*************************************************************************/
void triangle(int nr,int m1,int n12,float *uu,float *vv)
{
	int np,nq,i;

  /*  real pp(n12+nr-1), qq(n12+nr+nr-2), tt(n12) */
  /*  float pp[TRIS_NMAX], qq[TRIS_NMAX], tt[TRIS_NMAX]; */

	for (i=0;i<n12;i++)  qq[i] = uu[i*m1];

	if (n12 == 1) copy( n12, qq, tt);
	else {
		boxconv(nr, n12, qq, pp);      np = nr+n12-1;
		boxconv( nr, np , pp, qq);     nq = nr+np-1;
		for (i=0;i<n12;i++)  tt[i]  = qq[i+nr-1];
		for (i=0;i<nr-1;i++) tt[i]      +=qq[nr-i-2];      /* fold back near end */
		for (i=0;i<nr-1;i++) tt[n12-i-1]+=qq[n12+(nr-1)+i]; /* fold back far end */
	}
	for (i=0;i<n12;i++)  vv[i*m1]=tt[i];
	return;
}

/*************************************************************************
*                          Subroutine boxconv                            *
**************************************************************************
**************************************************************************
*   inputs:       nx,  xx(i), i=1,nx      the data                       *
*                 nb                      the box length                 *
*   output:       yy(i),i=1,nx+nb-1       smoothed data                  *
***************************************************************************/
void boxconv(int nb,int nx,float *xx,float *yy)
{
	int i,ny;

	/*  real bb(nx+nb) */
	/*    float bb[TRIS_NMAX]; */

	if( nb<1 || nb>nx) seperr("ERROR: in boxconv");
	ny = nx+nb-1;
	for (i=0;i<ny;i++) bb[i]=0.;

	bb[0] = xx[0];
	for (i=1;i<nx;i++)
		bb[i] = bb[i-1] + xx[i];         /*   make B(Z) = X(Z)/(1-Z) */
	for (i=nx;i<ny;i++)
		bb[i] = bb[i-1];
	for (i=0;i<nb;i++)
		yy[i] = bb[i];
	for (i=nb;i<ny;i++)
		yy[i] = bb[i] - bb[i-nb];        /*   make Y(Z) = B(Z)*(1-Z**nb) */
	for (i=0;i<ny;i++)
		yy[i] = yy[i]/nb;
}

/*************************************************************************
*                          Subroutine leaky                              *
**************************************************************************
*                  tridiagonal smoothing on 1-axis or 2-axis             *
**************************************************************************
*   distance,  input:  1. < distance < infinity                          *
*   uu[m1*n12]:        data in  is the vector (uu[m1*i], i=0,n12-1)      *
*   vv[m1*n12]:        data out is the vector (vv[m1*i], i=0,n12-1)      *
*************************************************************************/
void leaky(float distance,int m1,int n12,float *uu,float *vv)
{
	float a, b, dc, side;
	int i;
    
  /*  float vecin[TRIS_NMAX], vecout[TRIS_NMAX]; */
  /*  if (n12 > TRIS_NMAX) seperr("ERROR: n > TRIS_NMAX, recompile needed"); */

	a  = - (1.-1./distance);        b = 1.+a*a;     dc = b+a+a;
	a = a/dc;       b = b/dc;       side = a + b;

	for (i=0;i<n12;i++)  vecin[i] = uu[i*m1];

	if( distance<=1. || n12==1) copy( n12, vecin, vecout);
	else tris( n12, side, a, b, a, side, vecin, vecout);
  
	for (i=0;i<n12;i++)  vv[i*m1]=vecout[i];

	return;
}

void copy(int n,float *xx,float *yy)
{
	int i;

	for (i=0;i<n;i++) yy[i] = xx[i];

	return;
}

/*************************************************************************
*                          Subroutine tris                               *
**************************************************************************
*          tridiagonal simultaneous equations as in FGDP and IEI         *
**************************************************************************/

int tris(int n,float endl, float a, float b, float c, float endr,
    float *d, float *t)
{
	int i;

/*    float e[TRIS_NMAX], f[TRIS_NMAX], deni[TRIS_NMAX]; */
/*    if (n > TRIS_NMAX) seperr("ERROR: n > TRIS_NMAX, recompile needed"); */

	if( n == 1) { t[0] = d[0] / b; return 0; }
	e[0] = - a / endl;
	for (i=1;i<n-1;i++) {
		deni[i] = 1. / ( b + c * e[i-1] );
		e[i] = - a * deni[i];
	}

	f[0] = d[0] / endl;
	for (i=1;i<n-1;i++)
		f[i] = (d[i] - c * f[i-1]) * deni[i];

	t[n-1] = (d[n-1] - c * f[n-2])/(endr + c * e[n-2]);
	for (i=n-2;i>=0;i--)
		t[i] = e[i] * t[i+1] + f[i];

	return 0;
}
