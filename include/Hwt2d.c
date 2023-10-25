/*
=head1 NAME

Hwt3d - 2-D ray tracing

=head1 SYNOPSIS

< velocity.H Hwt2d par= > rays.H

=head1 DESCRIPTION

Does Huygens wavefront tracing.  
Produces a ray database.

=head1 INPUT PARAMETERS

=head2 SHOT LOCATION

=over 4

=item xsou - float

=item zsou - float

=back

=head2 RAY PARAMETERS

=item oT    - float

      [0.]

=item nT    - int

      [100] number of traveltime steps

=item dT    - float

      [0.01] traveltime sampling

=item oG    - float

      [0.]

=item nG    - int

      [90] number of shooting directions

=item dG    - float

      [1.0] shooting-angle sampling

=head1 SEE ALSO

L<Hwt3d>, L<FMeikonal>, L<hwt_travel_cube>, L<hwt_trace_rays> <Hwt3d>

=head1 CATEGORY
B<seis/travel>

=cut
>*/

#include <seplib.h>
#include <stdlib.h>
#ifndef booga
 typedef struct {
        double x;
        double z;
	double v;
} point;
#define booga djfkasf
#endif
#define YES 1
#define NO 0

#ifndef PI
#define PI            3.14159265358979323846
#endif

/* global variables and function declarations */

int nZ;		/* number of samples on the Z axis */
int nX; 	/* number of samples on the X axis */

float oZ;	/* origins */
float oX;

float dZ;	/* steps */
float dX;

int iZ; 	/* indices */
int iX;

float Zmax;	/* maximum Z */
float Xmax;	/* maximum X */

float** V;
point** p;
float* bound;

int nT;	/* T=time  */
int nG;	/* G=gamma=shooting angle */

float oT;
float oG;

float dT;
float dG;

int iT;
int iG;

float xsou,zsou;

#define sig(x)			x>=0?(x==0?0:1):-1
#define NO 0
#define YES 1

float get_xz_velocity(point);
float mixt_product(point, point, point, point);
void raytr(int,int);
void wfttr(int,int);
double distance(point,point);
int iscusp(point,point,point,point);
int read_cube(void);
int write_cube(void);
void compute(void);
void init_wf(void);

#define MY_SEP_DOC \
 sep_add_doc_line("NAME");\
 sep_add_doc_line("    Hwt3d - 2-D ray tracing");\
 sep_add_doc_line("");\
 sep_add_doc_line("SYNOPSIS");\
 sep_add_doc_line("    < velocity.H Hwt2d par= > rays.H");\
 sep_add_doc_line("");\
 sep_add_doc_line("DESCRIPTION");\
 sep_add_doc_line("    Does Huygens wavefront tracing. Produces a ray database.");\
 sep_add_doc_line("");\
 sep_add_doc_line("INPUT PARAMETERS");\
 sep_add_doc_line("  SHOT LOCATION");\
 sep_add_doc_line("    xsou - float");\
 sep_add_doc_line("    zsou - float");\
 sep_add_doc_line("");\
 sep_add_doc_line("  RAY PARAMETERS");\
 sep_add_doc_line("    oT - float");\
 sep_add_doc_line("          [0.]");\
 sep_add_doc_line("");\
 sep_add_doc_line("    nT - int");\
 sep_add_doc_line("          [100] number of traveltime steps");\
 sep_add_doc_line("");\
 sep_add_doc_line("    dT - float");\
 sep_add_doc_line("          [0.01] traveltime sampling");\
 sep_add_doc_line("");\
 sep_add_doc_line("    oG - float");\
 sep_add_doc_line("          [0.]");\
 sep_add_doc_line("");\
 sep_add_doc_line("    nG - int");\
 sep_add_doc_line("          [90] number of shooting directions");\
 sep_add_doc_line("");\
 sep_add_doc_line("    dG - float");\
 sep_add_doc_line("          [1.0] shooting-angle sampling");\
 sep_add_doc_line("");\
 sep_add_doc_line("SEE ALSO");\
 sep_add_doc_line("    Hwt3d, FMeikonal, hwt_travel_cube, hwt_trace_rays <Hwt3d>");\
 sep_add_doc_line("");\
 sep_add_doc_line("CATEGORY");\
 sep_add_doc_line("    seis/travel");\
 sep_add_doc_line("");
#include<sep.main>
MAIN()
{
                fprintf(stderr,"read velocity");
  read_cube();	fprintf(stderr," OK \n");
                fprintf(stderr,"initialize wavefronts");
  init_wf();	fprintf(stderr," OK \n");
  
                fprintf(stderr,"compute wavefronts ");
  compute();    fprintf(stderr," OK \n");
  
                fprintf(stderr,"write rays");
  write_cube(); fprintf(stderr," OK \n");
  
  return 1;
}

void compute(void)
{
  point P1,P2,P3,Q;
  float dd;

  for(iT=1;iT<nT-1;iT++) {
    for(iG=1;iG<nG-1;iG++) {
      P1=p[iT  ][iG-1];
      P2=p[iT  ][iG  ];    /* current 'middle' point */
      P3=p[iT  ][iG+1];
      Q =p[iT-1][iG  ];    /* previous 'middle' point on the ray */
      
      dd=distance(P3,P1);

      if(iscusp(Q,P1,P2,P3))
	raytr(iT,iG);
      else
	wfttr(iT,iG);
    }}
}

void init_wf(void)
{
  p=(point**)malloc(nT*sizeof(point*));
  for(iT=0;iT<nT;iT++)
    p[iT]=(point*) malloc(nG*sizeof(point)); 
  
  /* set all (x,z) to (0,0) */
  for(iT=0;iT<nT;iT++) {
    for(iG=0;iG<nG;iG++) {
      p[iT][iG].x=0.0;
      p[iT][iG].z=0.0;
      p[iT][iG].v=0.0;
    }}
  
  /* set (x,z) for iT=0 to (xsou,zsou) */
  iT=0;
  for(iG=0;iG<nG;iG++) {
    fprintf(stderr,"");
    p[iT][iG].x=xsou;
    p[iT][iG].z=zsou;
    p[iT][iG].v=get_xz_velocity(p[iT][iG]);
  }

  /* set (x,z) for iT=1 to (D sin(angle),D cos(angle)) */
  iT=1;
  for(iG=0;iG<nG;iG++) {
    double angle=PI*(oG+iG*dG)/180;
    double D;
    double Vcur;
    
    Vcur=p[0][iG].v;
    D=dT*Vcur;
    
    p[iT][iG].x=xsou+D*sin(angle);
    p[iT][iG].z=zsou+D*cos(angle);
    p[iT][iG].v=get_xz_velocity(p[1][iG]);
  }
  
  for(iT=1;iT<nT-1;iT++) {
    raytr(iT,0);
    raytr(iT,nG-1);
  }
}

int read_cube(void)
{
  int iD=0;
  int esize;
  float* data;
  
  if(0==hetch("esize","d",&esize)) esize=4;
  
  /* number of elements */
  if(0==hetch("n1","i",&nZ))  
    seperr("can not obtain n1 required for SEP datasets\n");
  if(0==hetch("n2","i",&nX))
    seperr("can not obtain n2 required for SEP datasets\n");
  
  /* origins */
  if(0==hetch("o1","f",&oZ))
    seperr("can not obtain o1 required for SEP datasets\n");
  if(0==hetch("o2","f",&oX))
    seperr("can not obtain o2 required for SEP datasets\n");
  
  /* steps */
  if(0==hetch("d1","f",&dZ))
    seperr("can not obtain d1 required for SEP datasets\n");
  if(0==hetch("d2","f",&dX))
    seperr("can not obtain d2 required for SEP datasets\n");
  
  Xmax=oX+(nX-1)*dX;
  Zmax=oZ+(nZ-1)*dZ;
  
  if(0==getch("xsou","f",&xsou)) xsou=0.;  
  if(0==getch("zsou","f",&zsou)) zsou=0.;
  
  if(0==getch("nT","d",&nT)) nT=100;  
  if(0==getch("nG","d",&nG)) nG=180;
  
  if(0==getch("oT","f",&oT)) oT=0;  
  if(0==getch("oG","f",&oG)) oG=0;
  
  if(0==getch("dT","f",&dT)){
  	if(0==getch("sT","f",&dT)) dT=0.01;  
	}

  if(0==getch("dG","f",&dG)){
  	if(0==getch("sG","f",&dG)) dG=0.01;  
		 dG=1.;
	}
  
  V=(float**) malloc(nZ*sizeof(float*));
  for(iZ=0;iZ<nZ;iZ++)
    V[iZ]=(float*) malloc(nX*sizeof(float));
  
  /* allocate a dumb vector of floats */
  data=(float*) malloc(nZ*nX*sizeof(float));
  if(nZ*nX*esize != sreed("in", data , nZ*nX*esize))
    seperr("trouble reading in data \n");
  
  /* transfer data into the velocity matrix */
  for(iX=0;iX<nX;iX++)
    for(iZ=0;iZ<nZ;iZ++,iD++)
      V[iZ][iX]=data[iD];
  free(data);
  
  return 0;
}

int write_cube(void)
{
  float *rayout;
  int index;
  int es=8;
  
  rayout=(float *) malloc (2*nG*nT*sizeof(float));
  
  index=0;
  for(iG=0;iG<nG;iG++) {
      for(iT=0;iT<nT;iT++) {
	  rayout[index++]=p[iT][iG].x;
	  rayout[index++]=p[iT][iG].z;
	}
    }
  
  if(0!=putch("esize","d",&es))
    seperr("trouble putting esize into output history file. \n");
  
  if(0!=putch("n1","d",&nT))
    seperr("trouble putting n1 into output history file. \n");
  
  if(0!=putch("o1","f",&oT))
    seperr("trouble putting o1 into output history file. \n");
  
  if(0!=putch("d1","f",&dT))
    seperr("trouble putting d1 into output history file. \n");
  
  if(0!=putch("n2","d",&nG))
    seperr("trouble putting n2 into output history file. \n");
  
  if(0!=putch("o2","f",&oG))
    seperr("trouble putting o2 into output history file. \n");
  
  if(0!=putch("d2","f",&dG))
    seperr("trouble putting d2 into output history file. \n");
  
  index=nT*nG;
  if(index*es != srite("out", rayout, index*es))
    seperr("trouble writing output data \n");	
  
  free(rayout);
  hclose();
  return 0;
}

void raytr(int itt, int igg)
{
  point P1,P2,P3,P4;
  double a,b,c;
  double R2;
  double ALPHA,BETA;
  point Q,S1,S2; 
  double DS1,DS2;
  double LL;
  double sina,cosa,tana;
  int ss,sa;

  P2=p[itt  ][igg  ];    /* current 'middle' point */
  Q =p[itt-1][igg  ];    /* previous 'middle' point on the ray */

  LL=sqrt((P2.x-Q.x)*(P2.x-Q.x)+(P2.z-Q.z)*(P2.z-Q.z));

  if(P2.z!=Q.z) {
    tana=(P2.x-Q.x)/(P2.z-Q.z);
    ss=(tana>=0)?1:-1;
    tana=(tana>=0)?tana:-tana;
    sina=tana/sqrt(1+tana*tana);
    cosa=1/sqrt(1+tana*tana);
  }
  else {
    cosa=0;
    sina=1;
    ss=1;
  }

  P1.x=P2.x-LL*cosa; P1.z=P2.z+ss*LL*sina; P1.v=get_xz_velocity(P1);
  P3.x=P2.x+LL*cosa; P3.z=P2.z-ss*LL*sina; P3.v=get_xz_velocity(P3);
  
  a= P1.x-P3.x    ; sa=(a>=0)?1:-1;
  b= P1.z-P3.z    ;
  c=(P1.v-P3.v)*dT;

  R2=P2.v*dT;
  ALPHA=c/(a*a+b*b);
  BETA=sa*sqrt(a*a+b*b-c*c)/(a*a+b*b);
  
  S1.x=P2.x-R2*(ALPHA*a-BETA*b);
  S1.z=P2.z-R2*(ALPHA*b+BETA*a);
  
  S2.x=P2.x-R2*(ALPHA*a+BETA*b);
  S2.z=P2.z-R2*(ALPHA*b-BETA*a);

  DS1=distance(S1,Q);
  DS2=distance(S2,Q);

  if(DS1>DS2) P4=S1;
  else        P4=S2;

  P4.v=get_xz_velocity(P4);
  p[itt+1][igg]=P4;
}

float get_xz_velocity(point P)
{
  int iXmin, iZmin, iXmax, iZmax;
  point P1,P2,P3,P4;
  double product;
  
  if(P.x<oX)   P.x=oX;
  if(P.x>Xmax) P.x=Xmax-dX;
  
  if(P.z<oZ)   P.z=oZ;
  if(P.z>Zmax) P.z=Zmax-dZ;
  
  iXmin=(int)((P.x-oX)/dX);	if(iXmin>nX-2) iXmax=nX-1; else iXmax=iXmin+1;
  iZmin=(int)((P.z-oZ)/dZ);	if(iZmin>nZ-2) iZmax=nZ-1; else iZmax=iZmin+1;
  /* 
     2 -> 3
     
     1 <- 4
  */
  P2.x=oX+iXmin*dX;	P3.x=oX+iXmax*dX;
  P2.z=oZ+iZmin*dZ;	P3.z=oZ+iZmin*dZ;
  P2.v=V[iZmin][iXmin];	P3.v=V[iZmin][iXmax];
  
  P1.x=oX+iXmin*dX;	P4.x=oX+iXmax*dX;
  P1.z=oZ+iZmax*dZ;	P4.z=oZ+iZmax*dZ;
  P1.v=V[iZmax][iXmin]; P4.v=V[iZmax][iXmax];
  
  product=(P.x-P1.x)*(P3.z-P1.z)-(P3.x-P1.x)*(P.z-P1.z);
  
  if(product>0)
    P.v=mixt_product(P,P1,P2,P3);	
  else
    P.v=mixt_product(P,P1,P3,P4);
  return P.v;
}

float mixt_product(point P, point P1, point P2, point P3)
{
  double mxv,mzv,mxz;
  double v;
  
  mxv=(P2.x-P1.x)*(P3.v-P1.v)-(P3.x-P1.x)*(P2.v-P1.v);
  mzv=(P2.z-P1.z)*(P3.v-P1.v)-(P3.z-P1.z)*(P2.v-P1.v);
  mxz=(P2.x-P1.x)*(P3.z-P1.z)-(P3.x-P1.x)*(P2.z-P1.z);
  
  v=P1.v+( (P.z-P1.z)*mxv-(P.x-P1.x)*mzv )/mxz;

  return v;
}

void wfttr(int itt, int igg)
{
  point P1,P2,P3,P4;
  double a,b,c;
  double R2;
  double ALPHA,BETA;
  point Q,S1,S2; 
  double DS1,DS2;
  int sa;

  P1=p[itt  ][igg-1];
  P2=p[itt  ][igg  ];    /* current 'middle' point */
  P3=p[itt  ][igg+1];
  Q =p[itt-1][igg  ];    /* previous 'middle' point on the ray */

  a= P1.x-P3.x    ;
  b= P1.z-P3.z    ;
  c=(P1.v-P3.v)*dT;

  sa=(a>=0)?1:-1;
  
  R2=P2.v*dT;
  ALPHA=c/(a*a+b*b);
  BETA=sa*sqrt(a*a+b*b-c*c)/(a*a+b*b); 
  
  S1.x=P2.x-R2*(ALPHA*a-BETA*b);
  S1.z=P2.z-R2*(ALPHA*b+BETA*a);
  
  S2.x=P2.x-R2*(ALPHA*a+BETA*b);
  S2.z=P2.z-R2*(ALPHA*b-BETA*a);
  
  DS1=distance(S1,Q);
  DS2=distance(S2,Q);
  
  if(DS1>DS2)
    P4=S1;
  else
    P4=S2;
  
  P4.v=get_xz_velocity(P4);
  
  p[itt+1][igg]=P4;
}

int iscusp(point Q, point P1, point P2, point P3)
{
  float j1,j3;
  int sj1,sj3;

  j1=(P2.x-P1.x)*(P2.z-Q.z)-(P2.x-Q.x)*(P2.z-P1.z); sj1=(j1>=0)?1:-1;
  j3=(P2.x-P3.x)*(P2.z-Q.z)-(P2.x-Q.x)*(P2.z-P3.z); sj3=(j3>=0)?1:-1;

  if (sj1*sj3==1)
    return YES;
  else
    return NO;
}

double distance(point A, point B)
{
  return (B.x-A.x)*(B.x-A.x)+(B.z-A.z)*(B.z-A.z);
}
