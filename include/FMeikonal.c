/*$

=head1 NAME

FMeikonal -Fast marching eikonal solver
  
=head1 SYNOPSIS

FMeikonal < VelocityCube.H >TTCube.H  pars
 
=head1 INPUT  PARAMETERS

=over 4

=item shotfile          - sepfile
     
      file with multiple shots (n2 - number of shots, n1=3)


=item zshot,yshot,xshot - real 

      [0., o2 + 0.5*(n2-1)*d2, o3 + 0.5*(n3-1)*d3 shot location

=item b1,b2,b3          - integer 

      [1,1,1] constant-velocity box around the source

=item br1,br2,br3       - real 

      [d1,d2,d3] constant-velocity box around the source


=item vel               - integer 

      [1]  velocity (1) or slowness (0)

=item vel0   - real

      velocity (or slowness) at the source [vel[0]]

=item order             - integer 

      [2]  first (1), second (2) or third (3) order

=back

=head1 DESCRIPTION  

Fast marching eikonal solver
 
=head1 SEE ALSO

L<Hwt3d>, L<fastmarch>, L<Gfgradz>

=head1 COMMENTS

Be careful with sources outside the domain.
You should smooth the velocity field if you want high-order accuracy.
Stability problems *may* occur if traveltime accuracy is beyond 
    machine precision.
The program uses constant velocity ray-tracing inside an initial box,
    and until it detects a velocity variation, whichever is greater.

=head1 CATEGORY

B<seis/travel>

=cut

>*/

/*
****************************************************************************
*  Copyright (c) 1997 Stanford Exploration Project                         * 
*  All Rights Reserved                                                     *
****************************************************************************
Author: Sergey Fomel
                          
second-order accuracy added by James Rickett (April 1999)
*/ 

#include <stdlib.h>
#include <math.h>
#include <sepmath.h>
#include <sepaux.h>
#include <septravel.h>
#include<sep.main>
#define MY_SEP_DOC \
 sep_add_doc_line("NAME");\
 sep_add_doc_line("    FMeikonal -Fast marching eikonal solver");\
 sep_add_doc_line("");\
 sep_add_doc_line("SYNOPSIS");\
 sep_add_doc_line("    FMeikonal < VelocityCube.H >TTCube.H pars");\
 sep_add_doc_line("");\
 sep_add_doc_line("INPUT  PARAMETERS");\
 sep_add_doc_line("    shotfile - sepfile");\
 sep_add_doc_line("              file with multiple shots (n2 - number of shots, n1=3)");\
 sep_add_doc_line("");\
 sep_add_doc_line("    zshot,yshot,xshot - real");\
 sep_add_doc_line("              [0., o2 + 0.5*(n2-1)*d2, o3 + 0.5*(n3-1)*d3 shot location");\
 sep_add_doc_line("");\
 sep_add_doc_line("    b1,b2,b3 - integer");\
 sep_add_doc_line("              [1,1,1] constant-velocity box around the source");\
 sep_add_doc_line("");\
 sep_add_doc_line("    br1,br2,br3 - real");\
 sep_add_doc_line("              [d1,d2,d3] constant-velocity box around the source");\
 sep_add_doc_line("");\
 sep_add_doc_line("    vel - integer");\
 sep_add_doc_line("              [1]  velocity (1) or slowness (0)");\
 sep_add_doc_line("");\
 sep_add_doc_line("    vel0 - real");\
 sep_add_doc_line("              velocity (or slowness) at the source [vel[0]]");\
 sep_add_doc_line("");\
 sep_add_doc_line("    order - integer");\
 sep_add_doc_line("              [2]  first (1), second (2) or third (3) order");\
 sep_add_doc_line("");\
 sep_add_doc_line("DESCRIPTION");\
 sep_add_doc_line("    Fast marching eikonal solver");\
 sep_add_doc_line("");\
 sep_add_doc_line("SEE ALSO");\
 sep_add_doc_line("    Hwt3d, fastmarch, Gfgradz");\
 sep_add_doc_line("");\
 sep_add_doc_line("COMMENTS");\
 sep_add_doc_line("    Be careful with sources outside the domain. You should smooth the");\
 sep_add_doc_line("    velocity field if you want high-order accuracy. Stability problems *may*");\
 sep_add_doc_line("    occur if traveltime accuracy is beyond machine precision. The program");\
 sep_add_doc_line("    uses constant velocity ray-tracing inside an initial box, and until it");\
 sep_add_doc_line("    detects a velocity variation, whichever is greater.");\
 sep_add_doc_line("");\
 sep_add_doc_line("CATEGORY");\
 sep_add_doc_line("    seis/travel");\
 sep_add_doc_line("");
MAIN(){
  int b1, b2, b3, n1, n2, n3, esize, vel, i, nshot, ndim, is,order;
  float s1, s2, s3, o1, o2, o3, d1, d2, d3, vel0, br1, br2, br3;
  float *t, *v, *s;
  char shotfile[20];

  if( fetch("n1","d",&n1) == 0 ) seperr("n1 must be supplied\n");
  if( fetch("n2","d",&n2) == 0 ) seperr("n2 must be supplied\n");
  if( fetch("n3","d",&n3) == 0 ) seperr("n3 must be supplied\n");
  if( fetch("esize","d",&esize) == 0 || esize != 4) 
    seperr("esize must be 4\n");

  if( fetch("d1","f",&d1) == 0 ) seperr("d1 must be supplied\n");
  if( fetch("d2","f",&d2) == 0 ) seperr("d2 must be supplied\n");
  if( fetch("d3","f",&d3) == 0 ) seperr("d3 must be supplied\n");
  if( fetch("o1","f",&o1) == 0 ) o1=0.; 
  if( fetch("o2","f",&o2) == 0 ) o2=0.;
  if( fetch("o3","f",&o3) == 0 ) o3=0.;

  if( getch("vel","d",&vel) == 0 ) vel=1;
  if( getch("order","d",&order) == 0 ) order=2;
  if((order > 3)||(order < 1)) order=2;

  if( getch("br1","f",&br1) == 0 ) br1=d1; 
  if( getch("br2","f",&br2) == 0 ) br2=d2; 
  if( getch("br3","f",&br3) == 0 ) br3=d3; 
  if( getch("b1","d",&b1) == 0 ) b1= (int) (br1/d1+0.5); 
  if( getch("b2","d",&b2) == 0 ) b2= (int) (br2/d2+0.5); 
  if( getch("b3","d",&b3) == 0 ) b3= (int) (br3/d3+0.5); 
  if( b1<1 ) b1=1;  if( b2<1 ) b2=1;  if( b3<1 ) b3=1;

/*   if( getch("b1","d",&b1) == 0 ) b1=1;  */
/*   if( getch("b2","d",&b2) == 0 ) b2=1; */
/*   if( getch("b3","d",&b3) == 0 ) b3=1; */

  putch ("b1","d",&b1);
  putch ("b2","d",&b2);
  putch ("b3","d",&b3);  

  if( getch("shotfile","s",shotfile) != 0) {
    if( auxpar("n1","d",&nshot,"shotfile") == 0 ) 
      seperr("n1 must be supplied in shotfile\n");
    if( auxpar("n2","d",&ndim,"shotfile") == 0  || ndim != 3) 
      seperr("n2 must be 3 in shotfile\n");
    if( auxpar("esize","d",&esize,"shotfile") == 0 || esize != 4) 
      seperr("esize must be 4 in shotfile\n");
  
    s = (float *) alloc (nshot*ndim*sizeof(float));
    sreed( "shotfile", s, nshot*ndim*esize);
    
    putch ("ndim","d",&ndim);
    putch ("n4","d",&nshot);
  } else {
    nshot = 1;
    ndim = 3;
    
    s = (float *) alloc (nshot*ndim*sizeof(float));     

    if( getch("zshot","f",&s[0]) == 0 ) s[0]=0.; 
    if( getch("yshot","f",&s[1]) == 0 ) s[1]=o2 + 0.5*(n2-1)*d2;
    if( getch("xshot","f",&s[2]) == 0 ) s[2]=o3 + 0.5*(n3-1)*d3;
    
    putch ("zshot","f",&s[0]);
    putch ("yshot","f",&s[1]);
    putch ("xshot","f",&s[2]);
  }

  t = (float *) alloc (n1*n2*n3*sizeof(float));
  v = (float *) alloc (n1*n2*n3*sizeof(float));

  sreed( "in", v, n1*n2*n3*esize);
  if (vel) {
    for (i=0;i<n1*n2*n3;i++)
      v[i] = 1./v[i];
  }

  if( getch("vel0","f",&vel0) == 0 ) {
    vel0=v[0];
  } else if (vel) {
    vel0 = 1./vel0;
  }

  putch ("slowness0","f",&vel0);
  hclose ();

  /* loop over shots */
  for( is = 0; is < nshot; is++) {
    s1 = s[is]         - o1; 
    s2 = s[is+nshot]   - o2; 
    s3 = s[is+2*nshot] - o3; 
    
    fastmarch(order,
	      s1,s2,s3,
	      b1,b2,b3,
	      n1,n2,n3,
	      d1,d2,d3,
	      vel0, v,t);
    srite ("out", t, n1*n2*n3*esize);
  }

  exit (0);
  return 1;
}

